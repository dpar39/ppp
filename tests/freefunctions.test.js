const init = require("../dist/index.cjs");

let api = null;
describe("free functions test suite", () => {
  beforeAll(async () => {
    api = await init();
  });

  afterAll(() => {
    api.terminate();
  });

  test("get wasm build time", async () => {
    const buildTimeStr = await api.getBuildTime();
    const buildTimeObj = new Date(buildTimeStr);
    expect(buildTimeObj).toBeInstanceOf(Date);
  });

  test("can log exceptions", async () => {
    try {
      await api.throwSampleException();
    } catch (e) {
      expect(e[0]).toBe("std::logic_error");
      expect(e[1]).toContain("A sample exception has been thrown");
    }
  });
});
