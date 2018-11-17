package org.pardinas.ppp;

import android.content.res.AssetManager;

import com.getcapacitor.JSObject;
import com.getcapacitor.NativePlugin;
import com.getcapacitor.Plugin;
import com.getcapacitor.PluginCall;
import com.getcapacitor.PluginMethod;

import java.io.IOException;
import java.io.InputStream;


import swig.libppp;


@NativePlugin()
public class PppPlugin extends Plugin {

    static {
        System.loadLibrary("libppp");
    }

    @PluginMethod()
    public void echo(PluginCall call) {
        String value = call.getString("value");

        JSObject ret = new JSObject();
        ret.put("value", "La vida es bella");
        call.success(ret);
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
    public void configure(PluginCall call) {

        String config = loadAsset("public/assets/config.bundle.json");

        boolean result = libppp.configure(config);

        if (result) {
            JSObject ret = new JSObject();
            ret.put("success", result);
            call.success(ret);
        }
    }

    @PluginMethod()
    public void setImage(PluginCall call) {

    }

    @PluginMethod()
    public void detectLandmarks(PluginCall call) {

    }

    @PluginMethod()
    public void createTilePrint(PluginCall call) {

    }
}
