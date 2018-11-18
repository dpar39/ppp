package org.pardinas.ppp;

import android.content.res.AssetManager;

import com.getcapacitor.JSObject;
import com.getcapacitor.NativePlugin;
import com.getcapacitor.Plugin;
import com.getcapacitor.PluginCall;
import com.getcapacitor.PluginMethod;

import java.io.IOException;
import java.io.InputStream;


import swig.PublicPppEngine;

import swig.libppp;


@NativePlugin()
public class PppPlugin extends Plugin {

    private PublicPppEngine engine;

    static {
        System.loadLibrary("libppp");
    }

    @Override()
    public void load() {
        engine = new PublicPppEngine();
        String config = loadAsset("public/assets/config.bundle.json");
        boolean result = engine.configure(config);
        if (!result)
        {
            engine = null;
        }
    }

    private String loadAsset(String filePath) {
        AssetManager assetManager = getActivity().getAssets();
        InputStream input;
        try {
            input = assetManager.open(filePath);

            int size = input.available();
            byte[] buffer = new byte[size];
            input.read(buffer);
            input.close();
            return new String(buffer);

        } catch (IOException e) {
            e.printStackTrace();
        }
        return new String();
    }

    @PluginMethod()
    public void setImage(PluginCall call) {
        String imgDataBase64 = call.getString("imgData");

        String imgKey = engine.setImage(imgDataBase64, 0).toString();
        if (imgKey != null && !imgKey.isEmpty()) {
            JSObject ret = new JSObject();
            ret.put("imgKey", imgKey);
            call.success(ret);
        }
        call.reject(null);
    }

    @PluginMethod()
    public void detectLandmarks(PluginCall call) {

    }

    @PluginMethod()
    public void createTilePrint(PluginCall call) {

    }
}
