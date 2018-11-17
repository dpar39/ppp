
declare global {
    interface PluginRegistry {
        PppPlugin?: PppPluginPlugin;
    }
}

export interface PppPluginPlugin {

    echo(options: { value: string }): Promise<{ value: string }>;

    configure(options: { cfg: string }): Promise<string>;

    setImage(options: { imgData: string }): Promise<{ imgKey: string }>;

    detectLandmarks(options: { imgKey: string }): Promise<{ landmarks: string }>;
}
