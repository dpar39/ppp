const crcTable = [];
function makeCRCTable() {
  if (crcTable) return crcTable;
  let c = 0;
  for (let n = 0; n < 256; n++) {
    c = n;
    for (let k = 0; k < 8; k++) {
      c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
    }
    crcTable[n] = c;
  }
  return crcTable;
}

export function crc32(bytes: Uint8Array) {
  const crcTable = makeCRCTable();
  let crc = 0 ^ -1;
  for (var i = 0; i < bytes.length; i++) {
    crc = (crc >>> 8) ^ crcTable[(crc ^ bytes[i]) & 0xff];
  }
  return (crc ^ -1) >>> 0;
}
