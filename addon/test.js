/////////////////////////////////////////////////
//               Configuration                 //
/////////////////////////////////////////////////
var engineConfigFile = 'config.json';
var inImgName = '../research/sample_test_images/000.jpg';
var outImgName = 'output.png';

var printDef = {
    canvas: {
        height: 4.0,
        width: 6.0,
        resolution: 300,
        units: "inch"
    },
    standard: {
        pictureWidth: 2.0,
        pictureHeight: 2.0,
        faceHeight: 1.1875,
        eyesHeight: 1.25,
        units: "inch"
    }
};
/////////////////////////////////////////////////

// var printDef = {
//     canvas: {
//         height: 5.0,
//         width: 7.0,
//         resolution: 300,
//         units: "inch"
//     },
//     standard: {
//         pictureWidth: 35,
//         pictureHeight: 45,
//         faceHeight: 34,
//         units: "mm"
//     }
// };


var fs = require("fs");
var addon = require("./addon");
// Create object under test
var pppEngine = new addon.PppWrapper();
// Read configuration json
var jsonConfig = fs.readFileSync(engineConfigFile, "utf8");
pppEngine.configure(jsonConfig);

// Try to set an Image
var imageData = fs.readFileSync(inImgName);

pppEngine.setImage(imageData, function(err, imgKey) {
    if (err) {
        throw err;
    }
    console.log('imgKey=' + imgKey);
    pppEngine.detectLandmarks(imgKey, function(err, landmarks) {
        if (err) {
            throw err;
        }
        console.log('landMarks=' + JSON.stringify(landmarks));

        // Copy the estimated crown and chin points 
        printDef.crownPoint = landmarks.crownPoint;
        printDef.chinPoint = landmarks.chinPoint;

        pppEngine.createTilePrint(imgKey, printDef, function(err, bufferData) {
            if (err) {
                throw err;
            }
            console.log('Print photo created!');
            fs.writeFile(outImgName, bufferData, function(err) {
                if (err) {
                    throw err;
                }
            });
        });
    });
});