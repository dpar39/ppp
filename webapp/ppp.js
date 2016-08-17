// Import the C++ addon

sessions = [];

var addon = require("./addon");
// This is our c++ wrapper object
var pppEngine = new addon.PppWrapper();

var detectLandMarks = function(imageData, response) {
    
    // Set the image in the library
    pppEngine.setImage(imageData, function(err0, imgKey){
        if (err0) {
            throw err0;
        }
        // Detect the land marks
        pppEngine.detectLandmarks(imgKey, function(err1, landmarks) {
        if (err1) {
            throw err1;
        }
    });

    var ppp = { item1: "item1val", item2: "item2val" };
    var json = JSON.stringify(landmarks);
  response.end(json);
} 