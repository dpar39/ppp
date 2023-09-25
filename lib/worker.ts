declare const self: Worker;
declare var pppWasmApi: any;

declare const process: any;
declare const require: any;

export default function (...args) {
  const debugLog = args[0] || true;
  const prefixOverride = args[1];
  function log(...args) {
    if (debugLog) {
      console.log('worker:', ...args);
    }
  }
  const handles = new Map();
  const api = {};
  function storeWasmObj(obj: any) {
    if (typeof obj === 'object' && '$$' in obj && 'ptr' in obj.$$) {
      const typename = obj.$$.ptrType.registeredClass.name;
      const handle = obj.$$.ptr;
      handles.set(handle, obj);
      log(`added object handle=${handle}, type=${typename} - handle count=${handles.size}`);
      return {
        __type: typename,
        __handle: handle,
      };
    }
    return obj;
  }
  function isHandle(handle: number) {
    return handles.has(handle);
  }
  function deleteObj(handle: number) {
    if (!isHandle(handle)) {
      return;
    }
    handles.get(handle).delete(); // deallocate the WASM object
    handles.delete(handle); // remove from the handles map
    log(`deleted object handle=${handle} - handle count=${handles.size}`);
  }
  function getWasmObj(handle: number) {
    if (handles.has(handle)) {
      return handles.get(handle);
    }
    return undefined;
  }

  function extractWasmApi(wasmApi: object) {
    const isFunc = (obj) => typeof obj === 'function';

    for (let m in wasmApi) {
      const mm = wasmApi[m];
      if (isFunc(mm) && 'name' in mm) {
        const clsHdl = mm.prototype;
        const objApi = {};
        if (typeof clsHdl === 'object') {
          for (let objMethodName in clsHdl) {
            if (!clsHdl.hasOwnProperty(objMethodName)) {
              continue;
            }
            const objMethodFunc = clsHdl[objMethodName];
            if (isFunc(objMethodFunc) && 'argCount' in objMethodFunc) {
              objApi[objMethodName] = objMethodFunc.argCount;
            }
          }
        }
        const ojbMethods = Object.keys(objApi);
        if (ojbMethods.length > 0) {
          api[m] = objApi;
        } else if ('argCount' in mm) {
          api[m] = mm.argCount;
        }
      }
    }
    return api;
  }

  log('initializing wasm worker ...');

  //_PPP_WASM_JS_INLINE_

  const moduleOpts = {
    locateFile: (path, prefix) => (prefixOverride || prefix) + path,
  };
  pppWasmApi(moduleOpts).then((wasmApi) => {
    const isBrowser = !(typeof process === 'object' && typeof require === 'function');
    const parentPort = isBrowser ? null : require('node:worker_threads').parentPort;
    function workerPostMessage(payload: object) {
      if (isBrowser) {
        self.postMessage(payload);
      } else {
        parentPort.postMessage({ data: payload });
      }
    }

    function onMessageReceived(event) {
      const methodName: string = event.data.method;
      const args = event.data.args;
      const callId = event.data.callId;
      const args2 = args.map((a) => (isHandle(a) ? getWasmObj(a) : a));

      log(`calling '${methodName}:${callId}' with ${args2.length} parameter(s)`);
      let result = undefined;
      let success = true;
      try {
        if (methodName.indexOf('$') > -1) {
          const [className, objMethodName] = methodName.split('$');
          if (objMethodName == 'create') {
            result = new (Function.prototype.bind.apply(
              wasmApi[className],
              [null].concat(args2)
            ))();
          } else if (objMethodName == 'delete') {
            deleteObj(args[0]);
          } else {
            const obj = args2[0];
            args2.shift();
            const expectedArgs = api[className][objMethodName];
            while (args2.length < expectedArgs) {
              args2.push(undefined);
            }
            result = obj[objMethodName].apply(obj, args2);
          }
        } else {
          // free function
          const expectedArgs = api[methodName];
          while (args2.length < expectedArgs) {
            args2.push(undefined);
          }
          result = wasmApi[methodName].apply(null, args2);
        }
        log(`success running ${methodName}:${callId}`);
      } catch (e) {
        log(`exception running '${methodName}:${callId}'`);
        result = wasmApi.getExceptionMessage(e);
        success = false;
      }
      workerPostMessage({
        result: storeWasmObj(result),
        callId: callId,
        success: success,
      });
    }

    const apiMethods = extractWasmApi(wasmApi);
    if (isBrowser) {
      self.onmessage = onMessageReceived;
    } else {
      parentPort.onmessage = onMessageReceived;
    }
    workerPostMessage({ initialized: true, api: apiMethods });
    log('worker ready has been reported');
  });
}
