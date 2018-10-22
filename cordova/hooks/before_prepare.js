var pluginId = 'cordova-plugin-cpp-bridge';

module.exports = function(context) {
    var path              = context.requireCordovaModule('path'),
        fs                = context.requireCordovaModule('fs'),
        cordova_util      = context.requireCordovaModule('cordova-lib/src/cordova/util');

    var projectRoot = cordova_util.cdProjectRoot();
    var cppDir = path.join(projectRoot, 'cpp');
    var cmakeListFile = path.join(cppDir, 'CMakeLists.txt');

    var templateCmakeLists = path.join(projectRoot, 'plugins',  pluginId, 'templates', 'CMakeLists.txt');

    var cmklstContent = fs.readFileSync(cmakeListFile, "utf-8");

    if (fs.existsSync(templateCmakeLists)) {
        var oldCmklstContent = fs.readFileSync(templateCmakeLists, "utf-8");
        if(oldCmklstContent != cmklstContent ) {
            console.log('[HOOK] Overriding CMakeLists.txt in "' + pluginId + '" plugin template');
            fs.writeFileSync(templateCmakeLists, cmklstContent);
        }
    }
};