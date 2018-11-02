package org.pardinas.ppp;

import com.getcapacitor.JSObject;
import com.getcapacitor.NativePlugin;
import com.getcapacitor.Plugin;
import com.getcapacitor.PluginCall;
import com.getcapacitor.PluginMethod;


import swig.libppp;


@NativePlugin()
public class PppPlugin extends Plugin {

  @PluginMethod()
  public void echo(PluginCall call) {
    String value = call.getString("value");

    JSObject ret = new JSObject();
    ret.put("value", "La vida es bella");
    call.success(ret);
  }

  @PluginMethod()
  public void configure(PluginCall call) {
    String config = call.getData().toString();
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
