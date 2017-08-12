
///////////////////////////////////////////////
// Server app
//////////////////////////////////////////////
console.log('Running node ' + process.version);
console.log('Initiating application ...');
var express = require('express');
var path = require('path');
var formidable = require('formidable');
var fs = require('fs');
var http = require('http');

/////////////////////////////////////////////
// Addon load and configuration
/////////////////////////////////////////////
try {
    var addon = require("./addon");
} catch (err) {
    console.log("Error importing addon :(");
    console.log(JSON.stringify(err));
    throw err;
}

var pppEngine = new addon.PppWrapper();
console.log('Addon instance created!');

// Configure addon engine instance
var engineConfigFile = 'config.json';
var jsonConfig = fs.readFileSync(engineConfigFile, "utf8");
pppEngine.configure(jsonConfig);
console.log('Addon instance configured!');

// App setup
var app = express();

// Serve the files out of ./public as our main files
//app.use(express.static(path.join(__dirname, 'public')));
app.use(express.static(path.join(__dirname, 'dist')));
app.use(express.static(path.join(__dirname, 'node_modules')));

var uploadImageDirectory = path.join(__dirname, '/uploads');

// App entry point
app.get('/', function (req, res) {
    res.sendfile('dist/index.html', { root: __dirname })
});

// -- Upload input image
app.post('/upload', function (req, res) {

    var form = new formidable.IncomingForm();
    // Every time a file has been uploaded successfully, rename it to it's orignal name
    form.on('fileBegin', function (field, file) {
        file.path = path.join(uploadImageDirectory, file.name);
    });

    form.on('file', function (field, file) {
         console.log('Image file uploaded successfully to server.')
        var imageData = fs.readFile(file.path, function(err, data) {
            // Set the image on the C++ addon
            pppEngine.setImage(data, function(err, imgKey) {
                if (err) {
                    console.log('Error setting image: \n' + err);
                    res.end(JSON.stringify({
                        'imgKey' : null,
                        'errorMsg' : str(err)
                    }));
                    return;
                } else {
                    console.log('imgKey=' + imgKey);
                    res.end(JSON.stringify({
                            'imgKey' : imgKey,
                    }));
                }
            });
        });
    });

    // log any errors that occur
    form.on('error', function (err) {
        console.log('An error has occured: \n' + err);
    });

    // Parse the incoming request containing the form data
    form.parse(req);
});

// -- Detect land marks
app.get('/landmarks', function (req, res) {
    console.log(req.url);
    var imgKey = req.query.imgKey;
    pppEngine.detectLandmarks(imgKey, function(err, landmarks) {
        if (err) {
            console.log('Error detecting landmarks for imgKey=' + imgKey + ':\n' + err);
            res.end(JSON.stringify({'error': err}));
            return;
        }
        var lm = JSON.stringify(landmarks);
        res.end(lm);
    });
});

function parsePoint(pt){
    var p = {
        x: parseInt(pt.x),
        y: parseInt(pt.y)
    }
    return p;
}

function validateUnits(units) {
    if (["mm", "cm", "inch"].indexOf(units) < 0) {
        throw "Unrecognized units: " + units;
    }
    return units;
}

function parseCanvas(canvas) {
    var obj = {
        height: parseFloat(canvas.height),
        width: parseFloat(canvas.width),
        resolution: parseInt(canvas.resolution),
        units: validateUnits(canvas.units)
    }
    return obj;
}

function parsePassportStandard(ps) {
    var obj = {
        pictureHeight: parseFloat(ps.pictureHeight),
        pictureWidth: parseFloat(ps.pictureWidth),
        faceHeight: parseFloat(ps.faceHeight),
        units: validateUnits(ps.units)
    }
    return obj;
}
function validatePrintRequest(printDef, imgKey) {

}

// -- Create photo print
app.get('/photoprint', function (req, res) {

    var imgKey = req.query.imgKey;
    var crownPoint = req.query.crownPoint;
    var chinPoint = req.query.chinPoint;
    var canvas = req.query.canvas;
    var standard = req.query.standard;
    var printDef = null;

    try {
        var printDef = {
            crownPoint : parsePoint(crownPoint),
            chinPoint : parsePoint(chinPoint),

            canvas : parseCanvas(canvas),
            standard : parsePassportStandard(standard)
        }
        console.log(printDef);
    } catch (e) {
        console.log(e);
    }

    console.log(imgKey);

    pppEngine.createTilePrint(imgKey, printDef, function(err, bufferData) {
         if (err) {
            console.log('Error detecting landmarks for imgKey=' + imgKey + ':\n' + err);
            res.end(JSON.stringify({'error': err}));
            return;
        }
        res.writeHead(200, {'Content-Type': 'image/png'});
        res.end(bufferData);
    });
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
