// rollup.config.js
import typescript from '@rollup/plugin-typescript';
import commonjs from '@rollup/plugin-commonjs';
import replace from '@rollup/plugin-replace';
import terser from '@rollup/plugin-terser';
import copy from 'rollup-plugin-copy';

import fs from 'fs';
import path from 'path';

import pkg from './package.json';

export default {
  input: 'lib/index.ts',
  output: [
    {
      file: pkg.exports['.'].import,
      format: 'esm',
      sourcemap: true,
    },
    {
      file: pkg.exports['.'].require,
      format: 'cjs',
      sourcemap: true,
    },
  ],
  plugins: [
    typescript({ outputToFilesystem: true }),
    commonjs(),
    replace({
      preventAssignment: true,
      include: ['lib/worker.ts'],
      delimiters: ['', ''],
      values: {
        '//_PPP_WASM_JS_INLINE_': () =>
          fs.readFileSync('bazel-bin/ppp-wasm-wrap/ppp-wasm.js', 'utf8'),
      },
    }),
    terser(),
    copy({
      targets: [
        { src: 'bazel-bin/ppp-wasm-wrap/ppp-wasm.wasm', dest: path.dirname(pkg.exports['.'].import) },
        { src: 'bazel-bin/ppp-wasm-wrap/ppp-wasm.data', dest: path.dirname(pkg.exports['.'].import) },
      ],
    }),
  ],
};
