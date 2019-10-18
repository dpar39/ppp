// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to use dlib's implementation of the paper:
        One Millisecond Face Alignment with an Ensemble of Regression Trees by
        Vahid Kazemi and Josephine Sullivan, CVPR 2014

    In particular, we will train a face landmarking model based on a small dataset
    and then evaluate it.  If you want to visualize the output of the trained
    model on some images then you can run the face_landmark_detection_ex.cpp
    example program with sp.dat as the input model.

    It should also be noted that this kind of model, while often used for face
    landmarking, is quite general and can be used for a variety of shape
    prediction tasks.  But here we demonstrate it only on a simple face
    landmarking task.
*/

#include <dlib/data_io.h>
#include <dlib/image_processing.h>
#include <iostream>
#include <rapidjson/document.h>
#include <unordered_set>

using namespace dlib;
using namespace std;

// ----------------------------------------------------------------------------------------

std::vector<std::vector<double>> get_interocular_distances(
    const std::vector<std::vector<full_object_detection>> & objects);
/*!
    ensures
        - returns an object D such that:
            - D[i][j] == the distance, in pixels, between the eyes for the face represented
              by objects[i][j].
!*/

// ----------------------------------------------------------------------------------------

std::set<int> g_removedLandmarksIndices;

void remove_landmarks(std::vector<std::vector<full_object_detection>> & facesTrain)
{
    unordered_set<int> ignoredLandmarks(g_removedLandmarksIndices.begin(), g_removedLandmarksIndices.end());
    auto selectedParts = std::vector<full_object_detection>();

    for (auto & input : facesTrain)
    {
        for (auto & p : input)
        {
            const auto n = static_cast<int>(p.num_parts());
            std::vector<point> parts;
            for (auto j = 0; j < n; ++j)
            {
                if (ignoredLandmarks.count(j + 1) > 0) // skip this landmark
                    continue;
                parts.push_back(p.part(j));
            }
            p = full_object_detection { p.get_rect(), parts };
        }
    }
}

int main(int argc, char ** argv)
{
    try
    {
        string thisCppFile = __FILE__;
        std::replace(thisCppFile.begin(), thisCppFile.end(), '\\', '/');
        auto found = thisCppFile.rfind('/');
        const auto thisDir = thisCppFile.substr(0, found);

        string facesDirectory, configJsonFilePath, outModelFilePath;
        if (argc == 1)
        {
            facesDirectory = "E:/Data/ibug_300W_large_face_landmark_dataset";
            configJsonFilePath = thisDir + "/config.json";
            outModelFilePath = thisDir + "/sp_model_new.dat";
        }
        else if (argc == 4)
        {
            cout << "Usage: trainer <dataset_dir> <config_json> <output_model>" << endl;
            facesDirectory = std::string(argv[1]);
            configJsonFilePath = std::string(argv[2]);
            outModelFilePath = std::string(argv[3]);
        }
        else
        {
            cout << "Usage: trainer <dataset_dir> <config_json> <output_model>" << endl;
            return 1;
        }

        cout << "Running model trainer using the following input:" << endl;
        cout << "   facesDirectory: " << facesDirectory << endl;
        cout << "   configJsonFilePath: " << configJsonFilePath << endl;
        cout << "   outModelFilePath: " << outModelFilePath << endl;
        rapidjson::Document config;

        {
            std::ifstream fs(configJsonFilePath, std::ios_base::in);
            const std::string configString((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
            config.Parse(configString.c_str());
        }

        auto array = config["shapePredictor"]["missingPoints"].GetArray();
        g_removedLandmarksIndices.clear();
        for (rapidjson::SizeType i = 0; i < array.Size(); i++)
        {
            g_removedLandmarksIndices.insert(array[i].GetInt());
        }

        cout << "Discarding " << g_removedLandmarksIndices.size() << " out of the 68 annotated landmarks" << endl;

        // The faces directory contains a training dataset and a separate
        // testing dataset.  The training data consists of 4 images, each
        // annotated with rectangles that bound each human face along with 68
        // face landmarks on each face.  The idea is to use this training data
        // to learn to identify the position of landmarks on human faces in new
        // images.
        //
        // Once you have trained a shape_predictor it is always important to
        // test it on data it wasn't trained on.  Therefore, we will also load
        // a separate testing set of 5 images.  Once we have a shape_predictor
        // created from the training data we will see how well it works by
        // running it on the testing images.
        //
        // So here we create the variables that will hold our dataset.
        // images_train will hold the 4 training images and faces_train holds
        // the locations and poses of each face in the training images.  So for
        // example, the image images_train[0] has the faces given by the
        // full_object_detections in faces_train[0].
        dlib::array<array2d<unsigned char>> imagesTrain, imagesTest;
        std::vector<std::vector<full_object_detection>> facesTrain, facesTest;

        // Now we load the data.  These XML files list the images in each
        // dataset and also contain the positions of the face boxes and
        // landmarks (called parts in the XML file).  Obviously you can use any
        // kind of input format you like so long as you store the data into
        // images_train and faces_train.  But for convenience dlib comes with
        // tools for creating and loading XML image dataset files.  Here you see
        // how to load the data.  To create the XML files you can use the imglab
        // tool which can be found in the tools/imglab folder.  It is a simple
        // graphical tool for labeling objects in images.  To see how to use it
        // read the tools/imglab/README.txt file.

        cout << "Loading training dataset ... please wait ..." << endl;
        load_image_dataset(imagesTrain, facesTrain, facesDirectory + "/labels_ibug_300W_train.orig.xml");
        // load_image_dataset(imagesTrain, facesTrain, facesDirectory + "/a.xml"); // testing
        remove_landmarks(facesTrain);

        cout << "Loading testing dataset ... please wait ..." << endl;
        load_image_dataset(imagesTest, facesTest, facesDirectory + "/labels_ibug_300W_test.orig.xml");
        remove_landmarks(facesTest);

        cout << "Dataset loaded. Starting to train the model ... please wait ..." << endl;

        // Now make the object responsible for training the model.
        shape_predictor_trainer trainer;
        // This algorithm has a bunch of parameters you can mess with.  The
        // documentation for the shape_predictor_trainer explains all of them.
        // You should also read Kazemi's paper which explains all the parameters
        // in great detail.  However, here I'm just setting three of them
        // differently than their default values.  I'm doing this because we
        // have a very small dataset.  In particular, setting the oversampling
        // to a high amount (300) effectively boosts the training set size, so
        // that helps this example.
        // trainer.set_oversampling_amount(300);
        // I'm also reducing the capacity of the model by explicitly increasing
        // the regularization (making nu smaller) and by using trees with
        // smaller depths.
        // trainer.set_nu(0.05);
        // trainer.set_tree_depth(2);

        // some parts of training process can be parallelized.
        // Trainer will use this count of threads when possible
        trainer.set_num_threads(8);

        // Tell the trainer to print status messages to the console so we can
        // see how long the training will take.
        trainer.be_verbose();

        // Now finally generate the shape model
        auto sp = trainer.train(imagesTrain, facesTrain);

        // Now that we have a model we can test it.  This function measures the
        // average distance between a face landmark output by the
        // shape_predictor and where it should be according to the truth data.
        // Note that there is an optional 4th argument that lets us rescale the
        // distances.  Here we are causing the output to scale each face's
        // distances by the interocular distance, as is customary when
        // evaluating face landmarking systems.
        cout << ">> Mean training error: "
             << test_shape_predictor(sp, imagesTrain, facesTrain, get_interocular_distances(facesTrain)) << endl;

        // The real test is to see how well it does on data it wasn't trained
        // on.  We trained it on a very small dataset so the accuracy is not
        // extremely high, but it's still doing quite good.  Moreover, if you
        // train it on one of the large face landmarking datasets you will
        // obtain state-of-the-art results, as shown in the Kazemi paper.
        cout << ">> Mean testing error:  "
             << test_shape_predictor(sp, imagesTest, facesTest, get_interocular_distances(facesTest)) << endl;

        // Finally, we save the model to disk so we can use it later.
        serialize(outModelFilePath) << sp;
        cout << "Model output written to " << outModelFilePath << ". DONE!" << endl;
    }
    catch (exception & e)
    {
        cout << "\nException thrown!" << endl;
        cout << "    " << e.what() << endl;
    }
}

// ----------------------------------------------------------------------------------------

int idx(const int idx68)
{
    const auto dist = std::distance(g_removedLandmarksIndices.begin(), g_removedLandmarksIndices.upper_bound(idx68));
    return idx68 - static_cast<int>(dist) - 1;
}

double interocular_distance(const full_object_detection & det)
{
    dlib::vector<double, 2> l, r;
    double cnt = 0;
    // Find the center of the left eye by averaging the points around
    // the eye.
    for (unsigned long i = 37; i <= 42; ++i)
    {
        l += det.part(idx(i));
        ++cnt;
    }
    l /= cnt;

    // Find the center of the right eye by averaging the points around
    // the eye.
    cnt = 0;
    for (unsigned long i = 43; i <= 48; ++i)
    {
        r += det.part(idx(i));
        ++cnt;
    }
    r /= cnt;

    // Now return the distance between the centers of the eyes
    return length(l - r);
}

std::vector<std::vector<double>> get_interocular_distances(
    const std::vector<std::vector<full_object_detection>> & objects)
{
    std::vector<std::vector<double>> temp(objects.size());
    for (unsigned long i = 0; i < objects.size(); ++i)
    {
        for (unsigned long j = 0; j < objects[i].size(); ++j)
        {
            temp[i].push_back(interocular_distance(objects[i][j]));
        }
    }
    return temp;
}
