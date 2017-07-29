var pluginId = 'cordova-plugin-cpp-bridge';

module.exports = function(context) {
    var path              = context.requireCordovaModule('path'),
        fs                = context.requireCordovaModule('fs'),
        cordova_util      = context.requireCordovaModule('cordova-lib/src/cordova/util');

    var projectRoot = cordova_util.cdProjectRoot();
    var cppDir = path.join(projectRoot, 'cpp');
    var cmakeListFile = path.join(cppDir, 'CMakeLists.txt');

    var templateCmakeLists = path.join(projectRoot, 'plugins',  pluginId, 'templates', 'CMakeLists.txt');
    console.log('>>> [HOOK] Overriding CMakeLists.txt in "' + pluginId + '" plugin template');
    fs.createReadStream(cmakeListFile).pipe(fs.createWriteStream(templateCmakeLists));
    fs.writeFileSync(templateCmakeLists, fs.readFileSync(cmakeListFile));
};