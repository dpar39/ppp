export interface PppEngine {

};

export interface PppApi {

  getBuildTime(): Promise<string>;

  terminate();
}
