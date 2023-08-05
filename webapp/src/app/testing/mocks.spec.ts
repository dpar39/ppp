const samplePhotoStandard = {
  id: 'gr_visa_us_photo',
  text: 'Greece Visa 2x2″ (from the USA)',
  country: 'Greece',
  docType: 'Visa',
  dimensions: {
    pictureWidth: 2.0,
    pictureHeight: 2.0,
    units: 'inch',
    dpi: 300.0,
    faceHeight: 1.29,
    bottomEyeLine: 1.18,
  },
  backgroundColor: '#ffffff',
  printable: true,
  officialLinks: ['http://www.mfa.gr/usa/en/services/services-for-non-greeks/visa-section.html'],
  comments: '',
};
export function createMockPhotoStandardService() {
  //   const spy = jasmine.createSpyObj(
  //     'PhotoStandardService',
  //     ['getCountryCode', 'getAllPhotoStandards', 'getSelectedStandard'],
  //     ['photoStandardSelected', 'croppedImageChanged']
  //   );
  //   spy.getCountryCode.and.returnValue('US');
  //   spy.getAllPhotoStandards.and.returnValue([]);

  //   spyOnProperty(spy, 'photoStandardSelected', 'get').and.callThrough();
  //   return spy;

  const mock = {
    getCountryCode: () => 'US',
    getAllPhotoStandards: () => [samplePhotoStandard],
    getSelectedStandard: () => samplePhotoStandard,
    photoStandardSelected: jasmine.createSpyObj('EventEmitter', ['subscribe']),
    croppedImageChanged: jasmine.createSpyObj('EventEmitter', ['subscribe']),
  };
  return mock;
}
