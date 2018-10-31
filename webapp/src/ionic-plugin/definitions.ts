declare global {
  interface PluginRegistry {
    PppPlugin?: PppPluginPlugin;
  }
}

export interface PppPluginPlugin {
  echo(options: { value: string }): Promise<{value: string}>;
}
