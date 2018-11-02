import { WebPlugin } from '@capacitor/core';
import { PppPluginPlugin } from './definitions';

export class PppPluginWeb extends WebPlugin implements PppPluginPlugin {
  async configure(cfg: string): Promise<string> {
    console.log('CONFIGURE', cfg);
    return Promise.resolve('OOOOO');
  }
  constructor() {
    super({
      name: 'PppPlugin',
      platforms: ['web']
    });
  }

  async echo(options: { value: string }): Promise<{value: string}> {
    console.log('ECHO', options);
    return Promise.resolve({ value: options.value });
  }
}

const PppPlugin = new PppPluginWeb();

export { PppPlugin };
