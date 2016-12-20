
var engineConfigFile = '../share/config.json';
var inImgName = '../research/sample_test_images/20161219_123451.jpg';
var outImgName = 'loira-4x6.png';

var fs = require("fs");

var addon = require("./addon");
// Create object under test
var pppEngine = new addon.PppWrapper();
// Read configuration json
var jsonConfig = fs.readFileSync(engineConfigFile, "utf8");
pppEngine.configure(jsonConfig);


// Try to set an Image
var imageData = fs.readFileSync(inImgName);

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

printdef = {
    canvas: {
        height: 4.0,
        width: 6.0,
        resolution: 300,
        units: "inch"
    },
    standard: {
        pictureWidth: 2,
        pictureHeight: 2,
        faceHeight: (1 + 1+3.0/8.0)/2,
        eyesHeight: (1+1.0/8.0 + 1+3.0/8.0)/2,
        units: "inch"
    }
};

pppEngine.setImage(imageData, function(err, imgKey) {
    if (err) {
        throw err;
    }
    console.log('imgKey=' + imgKey);
    pppEngine.detectLandmarks(imgKey, function(err, landmarks) {
        if (err) {
            throw err;
        }
        console.log(landmarks);

        printdef.crownPoint = landmarks.crownPoint;
        printdef.chinPoint = landmarks.chinPoint;

        printdef.crownPoint = {x: 574, y: 635};
        printdef.chinPoint = {x: 561, y: 1097};

        pppEngine.createTilePrint(imgKey, printdef, function(err, bufferData) {
            if (err) {
                throw err;
            }

            fs.writeFile(outImgName, bufferData, function(err) {
                if (err) {
                    return console.error(err);
                }
            });
        });
    });
});