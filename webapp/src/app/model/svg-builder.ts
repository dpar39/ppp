import { SafeResourceUrl } from '@angular/platform-browser';
import { crc32 } from './crc32';

function createElmt(elmtType: string) {
  const SVG_NS = 'http://www.w3.org/2000/svg';
  return document.createElementNS(SVG_NS, elmtType);
}

export class SvgBuilder {
  private _svg: SVGElement;

  constructor(width: number, height: number, fillColor: string) {
    const svg = createElmt('svg');

    const h = height.toString();
    const w = width.toString();
    svg.setAttribute('width', w);
    svg.setAttribute('height', h);
    svg.setAttribute('viewBox', `0 0 ${width} ${height}`);

    if (fillColor) {
      const background = createElmt('rect');
      background.setAttribute('width', w);
      background.setAttribute('height', h);
      background.setAttribute('fill', fillColor);
      svg.appendChild(background);
    }
    this._svg = svg;
  }

  getSvg() {
    return this._svg;
  }

  addImage(src: string | SafeResourceUrl, x: number, y: number, width: number, height: number) {
    const img = createElmt('image');
    img.setAttribute('width', width.toString());
    img.setAttribute('height', height.toString());
    img.setAttribute('x', x.toString());
    img.setAttribute('y', y.toString());
    img.setAttributeNS('http://www.w3.org/1999/xlink', 'xlink:href', src as string);
    this._svg.appendChild(img);
  }
}

export function renderAsPng(
  svgElmt: SVGSVGElement,
  dpi: number,
  callback: (blob: Blob) => void,
  blobUrl: string = null,
  dataUrl: string = null
) {
  const vb = svgElmt.viewBox.baseVal;
  const height = vb.height;
  const width = vb.width;
  let svgSerialized = new XMLSerializer().serializeToString(svgElmt);

  if (blobUrl && dataUrl) {
    svgSerialized = svgSerialized.split(blobUrl).join(dataUrl);
  }
  const svg = new Blob([svgSerialized], { type: 'image/svg+xml;charset=utf-8' });
  const url = URL.createObjectURL(svg);

  const img = new Image();
  img.onload = () => {
    const canvas = document.createElement('canvas');
    canvas.height = height;
    canvas.width = width;
    const ctx = canvas.getContext('2d');
    ctx.drawImage(img, 0, 0);
    URL.revokeObjectURL(url);
    canvas.toBlob((blob) => {
      if (dpi > 0) {
        addResolutionToPng(blob, dpi).then((b) => callback(b));
      } else {
        callback(blob);
      }
    });
  };
  img.src = url;
}

async function toArrayBuffer(blob) {
  return new Promise<ArrayBuffer>((resolve, reject) => {
    var reader = new FileReader();
    reader.onload = (event) => {
      resolve(event.target.result as ArrayBuffer);
    };
    reader.readAsArrayBuffer(blob);
  });
}

async function addResolutionToPng(blob: any, resolution_dpi: number) {
  var buffer = await toArrayBuffer(blob);
  let pngData = new Uint8Array(buffer);

  let physChunk = new ArrayBuffer(21);
  const dataView = new DataView(physChunk);
  let idx = 0;
  dataView.setUint32(idx, 9, false); // Chunk length
  idx += 4;

  for (const ch of ['p', 'H', 'Y', 's']) {
    dataView.setUint8(idx++, ch.charCodeAt(0));
  }

  const pixPerMeter = Math.round((resolution_dpi * 1000.0) / 25.4);
  dataView.setUint32(idx, pixPerMeter, false);
  idx += 4;
  dataView.setUint32(idx, pixPerMeter, false);
  idx += 4;
  dataView.setUint8(idx++, 1); // units are pixel per meter

  const crc = crc32(new Uint8Array(physChunk, 4, 13));
  dataView.setUint32(idx, crc, false);

  const iDat = ['I', 'D', 'A', 'T'].map((x) => x.charCodeAt(0));
  let iDatIndex = -1;
  for (let i = 0; i < pngData.length; ++i) {
    let found = true;
    for (let j = 0; j < iDat.length; ++j) {
      if (pngData[i + j] != iDat[j]) {
        found = false;
        break;
      }
    }
    if (found) {
      iDatIndex = i;
      break;
    }
  }

  let splitPos = iDatIndex - 4;
  const pngDataWithDpi = new Uint8Array(buffer.byteLength + physChunk.byteLength);
  pngDataWithDpi.set(new Uint8Array(buffer, 0, splitPos), 0);
  pngDataWithDpi.set(new Uint8Array(physChunk), splitPos);
  const iEnd = new Uint8Array(buffer, splitPos, buffer.byteLength - splitPos);
  pngDataWithDpi.set(iEnd, splitPos + physChunk.byteLength);
  return new Blob([pngDataWithDpi]);
}
