if( 'function' === typeof importScripts) {
    // This loads the wasm generated glue code
    importScripts('libppp.js');


    addEventListener('message', (e) =>{
        switch(e.data.cmd) {
            case "setImage":
                setImage(e.data.imageData);
                break;

            case "detectLandmarks":
                detectLandmarks(e.data.imgKey);
                break;
        }
    });


    function _stringToPtr(str) {
        const str_len = lengthBytesUTF8(str);
        let ptr = Module._malloc(str_len + 1);
        stringToUTF8(str, ptr, str_len + 1);
        return ptr;
    }

     // Overrides for the generated emcc script, module gets redifined later
    Module.onRuntimeInitialized = () => {
        fetch('config.bundle.json')
        .then(res => res.text())
        .then((config) => {
            ptr = _stringToPtr(config);
            Module._configure(ptr);
            Module._free(ptr);
            postMessage({'cmd': 'onRuntimeInitilized'});
        });

    };

    function _arrayToHeap(typedArray) {
         const numBytes = typedArray.length * typedArray.BYTES_PER_ELEMENT;
         const ptr = Module._malloc(numBytes);
         let heapBytes = Module.HEAPU8.subarray(ptr, ptr + numBytes);
         heapBytes.set(typedArray);
         return [ptr, numBytes];
    }

    function setImage(imageDataArrayBuf) { // ArrayBuffer
        const imageData = new Uint8Array(imageDataArrayBuf);
        let [imagePtr, numBytes] = _arrayToHeap(imageData);
        const imgKeyPtr = Module._malloc(16);

        const success = Module._set_image(imagePtr, numBytes, imgKeyPtr);

        const imgKey = Pointer_stringify(imgKeyPtr);
        Module._free(imgKeyPtr);
        Module._free(imagePtr);
        postMessage({'cmd': 'onImageSet', 'imgKey' : imgKey });
    }

    function detectLandmarks(imgKey) {

        const imgKeyPtr = _stringToPtr(imgKey);
        const landMarksPtr = Module._malloc(1000000);

        const success = Module._detect_landmarks(imgKeyPtr, landMarksPtr);

        const landMarksStr = Pointer_stringify(landMarksPtr);
        Module._free(imgKeyPtr);
        Module._free(landMarksStr);
        postMessage({'cmd': 'onLandmarksDetected', 'landmarks' : JSON.parse(landMarksStr)});
    }
}
