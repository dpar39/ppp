
var fs = require("fs");

var addon = require("./addon");

// Create object under test
var pppEngine = new addon.PppWrapper();


var engineConfigFile = '../share/config.json';
// Read configuration json
var jsonConfig = fs.readFileSync(engineConfigFile, "utf8");
pppEngine.configure(jsonConfig);


// Try to set an Image
var imageData = fs.readFileSync("../research/sample_test_images/000.jpg");

var printdef = {
    canvas: {
        height: 5.0,
        width: 7.0,
        resolution: 300,
        units: "inch"
    },
    standard: {
        pictureWidth: 35,
        pictureHeight: 45,
        faceHeight: 34,
        units: "mm"
    }
};

pppEngine.setImage(imageData, function(err) {
    if (err) {
        throw err;
    }
    pppEngine.detectLandmarks(function(err1, landmarks) {
        if (err1) {
            throw err1;
        }
        console.log(landmarks);

        printdef.crownPoint = landmarks.crownPoint;
        printdef.chinPoint = landmarks.chinPoint;

        pppEngine.createTilePrint(printdef, function(err2, bufferData) {
            if (err2) {
                throw err2;
            }
            console.log(bufferData);
            fs.writeFile("test.png", bufferData, function(err3) {
                if (err3) {
                    return console.error(err3);
                }
            });
        });
    });
});


// var engine = addon.createEngine();
// engine.configure();
// var sampleImage = fs.readFile()
// engine.setImage()