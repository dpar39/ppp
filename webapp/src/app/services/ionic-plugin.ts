
declare global {
    interface PluginRegistry {
        PppPlugin?: PppPluginPlugin;
    }
}

export interface PppPluginPlugin {

    setImage(options: { imgData: string }): Promise<{ imgKey: string }>;

    detectLandmarks(options: { imgKey: string }): Promise<{ landmarks: string }>;
}
