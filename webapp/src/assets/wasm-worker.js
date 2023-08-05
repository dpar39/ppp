if ('function' === typeof importScripts) {
  let pppEngine = null;
  self.onWasmFetchSuccess = (url, numBytes, totalBytes) => {
    if (pppEngine.isConfigured()) {
      postMessage({
        cmd: 'onAppDataLoadingProgress',
        progress: 1.0,
        step: 'Application ready',
      });
      postMessage({ cmd: 'onRuntimeInitialized' });
    }
  };

  self.onWasmFetchProgress = (url, numBytes, totalBytes) => {
    const urlStr = UTF8ToString(url);
    if (urlStr.endsWith('sp_model.dat') && numBytes > 0 && totalBytes > 0) {
      postMessage({
        cmd: 'onAppDataLoadingProgress',
        progress: 0.25 + (numBytes / totalBytes) * 0.7,
        step: 'Downloading models ...',
      });
    }
  };

  postMessage({
    cmd: 'onAppDataLoadingProgress',
    progress: 0.1,
    step: 'Warming up ...',
  });

  importScripts('ppp-wasm.js');

  postMessage({
    cmd: 'onAppDataLoadingProgress',
    progress: 0.2,
    step: 'Compiling wasm code ...',
  });

  addEventListener('message', (e) => {
    switch (e.data.cmd) {
      case 'setImage':
        setImage(e.data.imageData);
        break;
      case 'detectLandmarks':
        detectLandmarks(e.data.imgKey);
        break;
      case 'createTiledPrint':
        createTilePrint(e.data.request);
        break;
    }
  });

  function _stringToPtr(str) {
    const str_len = lengthBytesUTF8(str);
    let ptr = Module._malloc(str_len + 1);
    stringToUTF8(str, ptr, str_len + 1);
    return ptr;
  }

  // Overrides for the generated emcc script, module gets redefined later
  Module.onRuntimeInitialized = async () => {
    Module.InitGoogleLogging();
    pppEngine = new Module.Engine();
    postMessage({
      cmd: 'onAppDataLoadingProgress',
      progress: 0.5,
      step: 'Loading configuration ...',
    });
    let res = await fetch('/assets/config.json');
    const configJson = await res.text();
    pppEngine.configure(configJson);
    if (pppEngine.isConfigured()) {
      postMessage({
        cmd: 'onAppDataLoadingProgress',
        progress: 1.0,
        step: 'Application ready',
      });
      postMessage({ cmd: 'onRuntimeInitialized' });
    }
  };

  Module.print = (msg) => {
    console.log(msg);
  };

  function _arrayToHeap(typedArray) {
    const numBytes = typedArray.length * typedArray.BYTES_PER_ELEMENT;
    const ptr = Module._malloc(numBytes);
    let heapBytes = Module.HEAPU8.subarray(ptr, ptr + numBytes);
    heapBytes.set(typedArray);
    return [ptr, numBytes];
  }

  async function setImage(imageDataArrayBuf) {
    const imageData = new Uint8Array(imageDataArrayBuf);
    const metadata = pppEngine.setImage(imageData);
    const imageMetadata = JSON.parse(metadata);
    let cvMat = pppEngine.getImage(imageMetadata.imgKey);
    const imgBuf = cvMat.data.slice(0);
    postMessage(
      {
        cmd: 'onImageSet',
        imgKey: imageMetadata.imgKey,
        EXIFInfo: imageMetadata.EXIFInfo,
        imageBuffer: imgBuf.buffer,
        width: cvMat.cols,
        height: cvMat.rows,
      },
      [imgBuf.buffer]
    );
    cvMat.delete();
  }

  function detectLandmarks(imgKey) {
    const landMarksStr = pppEngine.detectLandmarks(imgKey);
    postMessage({ cmd: 'onLandmarksDetected', landmarks: JSON.parse(landMarksStr) });
  }
}
