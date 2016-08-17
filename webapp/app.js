
///////////////////////////////////////////////
// Server app
//////////////////////////////////////////////
var express = require('express');
var path = require('path');
var formidable = require('formidable');
var fs = require('fs');

var app = express();

// Serve the files out of ./public as our main files
app.use(express.static(path.join(__dirname, 'public')));

var uploadImageDirectory = path.join(__dirname, '/uploads');

// App entry point
app.get('/', function (req, res) {
    res.sendfile('public/index.html', { root: __dirname })
});

app.post('/upload', function (req, res) {

    var form = new formidable.IncomingForm();
    form.uploadDir = uploadImageDirectory;
    // every time a file has been uploaded successfully,
    // rename it to it's orignal name
    form.on('file', function (field, file) {
        fs.rename(file.path, path.join(form.uploadDir, file.name));
    });
    // log any errors that occur
    form.on('error', function (err) {
        console.log('An error has occured: \n' + err);
    });

    // once all the files have been uploaded, send a response to the client
    form.on('end', function () {
        var a = { "A": "a", "B": 1 };

        // TODO: set the image and respond with the imageKey
        var imageKey = "123456";
        res.end(imageKey);
    });

    // parse the incoming request containing the form data
    form.parse(req);
});

app.get('/landmarks', function (req, res) {

    var landmarks = {
        crownPoint: {
            x: 1250,
            y: 1073
        },
        chinPoint: {
            x: 1191,
            y: 2007
        }
    };
    req.end(JSON.stringify(landmarks));
});

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
