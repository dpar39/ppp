if ('function' === typeof importScripts) {
    // This loads the wasm generated glue code
    importScripts('libppp.js');

    addEventListener('message', e => {
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

    const expectedConfigTimeMilliseconds = 3000;

    function _retrieveApplicationData() {
        const xhr = new XMLHttpRequest();
        let lastProgress = 0;

        return new Promise((resolve, reject) => {
            const start = new Date();
            xhr.open('GET', 'config.bundle.json');
            xhr.onprogress = oEvent => {
                if (oEvent.lengthComputable) {
                    const elapsedTimeMs = new Date() - start;
                    if (elapsedTimeMs < 1 || oEvent.total <= 0) {
                        return;
                    }
                    const expectedTotalTimeMs =
                        oEvent.loaded / oEvent.total / elapsedTimeMs + expectedConfigTimeMilliseconds;
                    const progress = elapsedTimeMs / expectedTotalTimeMs;

                    if (progress > lastProgress) {
                        postMessage({
                            cmd: 'onAppDataLoadingProgress',
                            progressPct: progress * 100,
                            step: 'Loading config'
                        });
                        lastProgress = progress;
                    }
                } else {
                    console.error('Unable to provide download progress');
                    alert('Unable to download application data from the server. Try reloading the page');
                    reject();
                }
            };
            xhr.onload = e => {
                postMessage({
                    cmd: 'onAppDataLoadingProgress',
                    progressPct: lastProgress* 100,
                    step: 'Initializing ...'
                });

                resolve(xhr.responseText);
            };
            xhr.send();
        });
    }

    // Overrides for the generated emcc script, module gets redifined later
    Module.onRuntimeInitialized = () => {
        _retrieveApplicationData().then(config => {
            ptr = _stringToPtr(config);
            Module._configure(ptr);
            Module._free(ptr);
            postMessage({cmd: 'onRuntimeInitialized'});
        });
    };

    function _arrayToHeap(typedArray) {
        const numBytes = typedArray.length * typedArray.BYTES_PER_ELEMENT;
        const ptr = Module._malloc(numBytes);
        let heapBytes = Module.HEAPU8.subarray(ptr, ptr + numBytes);
        heapBytes.set(typedArray);
        return [ptr, numBytes];
    }

    function setImage(imageDataArrayBuf) {
        // ArrayBuffer
        const imageData = new Uint8Array(imageDataArrayBuf);
        let [imagePtr, numBytes] = _arrayToHeap(imageData);
        const imgKeyPtr = Module._malloc(16);

        const success = Module._set_image(imagePtr, numBytes, imgKeyPtr);

        const imgKey = UTF8ToString(imgKeyPtr, numBytes);
        Module._free(imgKeyPtr);
        Module._free(imagePtr);
        postMessage({cmd: 'onImageSet', imgKey: imgKey});
    }

    function detectLandmarks(imgKey) {
        const imgKeyPtr = _stringToPtr(imgKey);
        const landMarksPtr = Module._malloc(1000000);

        const success = Module._detect_landmarks(imgKeyPtr, landMarksPtr);

        const landMarksStr = UTF8ToString(landMarksPtr, 1000000);
        Module._free(imgKeyPtr);
        Module._free(landMarksStr);
        postMessage({cmd: 'onLandmarksDetected', landmarks: JSON.parse(landMarksStr)});
    }

    function createTilePrint(requestObject) {
        const imgKeyPtr = _stringToPtr(requestObject.imgKey);
        const requestObjPtr = _stringToPtr(JSON.stringify(requestObject));

        const outImageDataPtr = Module._malloc(10000000);
        const imageDataSize = Module._create_tiled_print(imgKeyPtr, requestObjPtr, outImageDataPtr);

        let heapBytes = Module.HEAPU8.subarray(outImageDataPtr, outImageDataPtr + imageDataSize);

        Module._free(imgKeyPtr);
        Module._free(requestObjPtr);
        postMessage({cmd: 'onCreateTilePrint', pngData: heapBytes});
        Module._free(outImageDataPtr);
    }
}
