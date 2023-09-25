import workerInitialize from './worker';
import { PppApi } from './types';

declare const process: any;
declare const require: any;

// Make sure only one web worker instance is created
// regardless of how many times package initialization is requested
type InitCallback = (v3d: PppApi) => void;
let workerInitializing = false;
let resolvedApi: PppApi;
let pendingInitRequests: InitCallback[] = [];

export default function (...args): Promise<PppApi> {
  const debugLog = args[0] || true;
  const profileLog = args[1] || false;
  function debug(...args) {
    if (debugLog) {
      console.log('main:', ...args);
    }
  }

  function initWebWorker() {
    workerInitializing = true;
    const isBrowser = !(typeof process === 'object' && typeof require === 'function');
    const startTime = new Date().valueOf();
    debug('starting new web worker...');

    const prefixOverride = isBrowser
      ? window.location.href.replace(/(.+\w\/)(.+)/, '/$2')
      : __dirname + '/';
    let worker: any;
    const workerCode = `(${workerInitialize.toString().trim()})(${debugLog},"${prefixOverride}");`;
    if (isBrowser) {
      const objectUrl = URL.createObjectURL(new Blob([workerCode], { type: 'text/javascript' }));
      worker = new window.Worker(objectUrl);
      URL.revokeObjectURL(objectUrl);
    } else {
      console.log(`${__dirname}`);
      const { Worker } = require('node:worker_threads');
      worker = new Worker(workerCode, { eval: true });
    }

    let callId = 0;
    const pendingPromises = new Map();
    let asyncApi: PppApi | object;

    function makeAsyncFunction(methodName: string) {
      return async function (...args: any) {
        if (typeof this === 'object' && this.hasOwnProperty('__handle')) {
          args.unshift(this.__handle);
        }
        return new Promise((accept, reject) => {
          const currentId = callId++;
          debug(`calling ${methodName}:${currentId}`);
          pendingPromises.set(currentId, [accept, reject, methodName, new Date()]);
          worker.postMessage({
            method: methodName,
            args: args.map((a) =>
              typeof a === 'object' && a.hasOwnProperty('__handle') ? a.__handle : a
            ),
            callId: currentId,
          });
        });
      };
    }

    function makeAsyncApi(api: object) {
      asyncApi = {
        terminate: () => worker.terminate(),
      };
      for (let name in api) {
        if (typeof api[name] == 'object') {
          const className = name;
          asyncApi[className] = {
            create: makeAsyncFunction(`${className}$create`),
            delete: makeAsyncFunction(`${className}$delete`),
          };
          for (let methodName in api[className]) {
            asyncApi[className][methodName] = makeAsyncFunction(`${className}$${methodName}`);
          }
        } else {
          asyncApi[name] = makeAsyncFunction(name);
        }
      }
      return asyncApi;
    }

    function workerOnMessage(e) {
      if (!e.data) {
        return;
      }
      if (e.data.initialized && e.data.api) {
        debug('worker has been initialized and ready');
        makeAsyncApi(e.data.api);
        debug(`worker initialized after ${new Date().valueOf() - startTime}ms`);
        resolvedApi = asyncApi as PppApi;
        for (let resolve of pendingInitRequests) {
          resolve(resolvedApi);
        }
        pendingInitRequests = [];
      } else {
        const result = e.data.result;
        if (
          typeof result === 'object' &&
          result.hasOwnProperty('__type') &&
          asyncApi.hasOwnProperty(result.__type)
        ) {
          result.__proto__ = asyncApi[result.__type];
        }
        const callId = e.data.callId;
        const [acceptCb, rejectCb, methodName, startTime] = pendingPromises.get(callId);
        if (profileLog) {
          const elapsedMs = new Date().valueOf() - startTime;
          console.log(`main: '${methodName}:${callId}' took ${elapsedMs}ms`);
        }
        if (e.data.success) {
          acceptCb(e.data.result);
        } else {
          rejectCb(e.data.result);
        }
        pendingPromises.delete(e.data.callId);
      }
    }

    if (isBrowser) {
      worker.onmessage = workerOnMessage;
    } else {
      worker.on('message', workerOnMessage);
    }
  }
  return new Promise((resolve) => {
    if (resolvedApi) {
      resolve(resolvedApi);
    } else {
      pendingInitRequests.push(resolve);
      if (!workerInitializing) {
        initWebWorker();
      }
    }
  });
}
export * from './types';
