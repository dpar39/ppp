package org.pardinas.ppp;

import android.content.res.AssetManager;

import com.getcapacitor.JSObject;
import com.getcapacitor.NativePlugin;
import com.getcapacitor.Plugin;
import com.getcapacitor.PluginCall;
import com.getcapacitor.PluginMethod;


import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

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

  @PluginMethod()
  public void configure(PluginCall call) {
    String config1 = call.getString("cfg");

    AssetManager assetManager = getContext().getAssets();

call.getObject("cfg")
    InputStream input;
    String config  = "";

    try
    {
      input = assetManager.open("config.json");
      int size = input.available();
      byte[] buffer = new byte[size];
      input.read(buffer);
      input.close();
      // byte buffer into a string
      config = new String(buffer);

    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }





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
