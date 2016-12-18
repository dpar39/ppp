
///////////////////////////////////////////////
// Server app
//////////////////////////////////////////////
var express = require('express');
var path = require('path');
var formidable = require('formidable');
var fs = require('fs');

/////////////////////////////////////////////
// Addon load and configuration
/////////////////////////////////////////////
var addon = require("./addon");
var pppEngine = new addon.PppWrapper();

var engineConfigFile = '../share/config.json';
// Read configuration json
var jsonConfig = fs.readFileSync(engineConfigFile, "utf8");
pppEngine.configure(jsonConfig);


// App setup
var app = express();

// Serve the files out of ./public as our main files
app.use(express.static(path.join(__dirname, 'public')));

var uploadImageDirectory = path.join(__dirname, '/uploads');

// App entry point
app.get('/', function (req, res) {
    res.sendfile('public/index.html', { root: __dirname })
});


// -- Upload input image
app.post('/upload', function (req, res) {

    var form = new formidable.IncomingForm();
    // Every time a file has been uploaded successfully, rename it to it's orignal name
    form.on('file', function (field, file) {
        fs.rename(file.path, path.join(uploadImageDirectory, file.name));
    });
    // log any errors that occur
    form.on('error', function (err) {
        console.log('An error has occured: \n' + err);
    });

    // once all the files have been uploaded, send a response to the client
    form.on('end', function () {
        // Set the image on the C++ addon
        pppEngine.setImage(imageData, function(err, imgKey) {
            if (err) {
                console.log('Error setting image: \n' + err);
                return;
            }
            console.log('imgKey=' + imgKey);
            res.end(imageKey);
        });
    });

    // Parse the incoming request containing the form data
    form.parse(req);
});

// -- Detect land marks
app.get('/landmarks', function (req, res) {

    // TODO: Get the image key from the URL
    imgKey = '12345678';
    // var landmarks = {
    //     crownPoint: {
    //         x: 1250,
    //         y: 1073
    //     },
    //     chinPoint: {
    //         x: 1191,
    //         y: 2007
    //     }
    // };

    pppEngine.detectLandmarks(imgKey, function(err, landmarks) {
        if (err) {
            console.log('Error detecting landmarks for imgKey=' + imgKey + ':\n' + err);
            return;
        }
        req.end(JSON.stringify(landmarks));
    });
});

// -- Create photo print
app.get('/photoprint', function (req, res) {

});

// Create upload directory if it doesn't exist
if (!fs.existsSync(uploadImageDirectory)) {
    fs.mkdirSync(uploadImageDirectory);
}

// start server on the specified port and binding host
var port = process.env.PORT || 3000;
app.listen(port, function () {
    console.log("Server listening on port " + port);
});
