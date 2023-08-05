import { getTestBed, TestBed, waitForAsync } from '@angular/core/testing';

import { PhotoStandardService } from './photo-standard.service';
import { HttpClientTestingModule, HttpTestingController } from '@angular/common/http/testing';
import { StorageService } from './storage.service';

const COUNTRY_CODE = 'countryCode';
const LAST_SELECTED = 'lastSelected';

describe('PhotoStandardService', () => {
  let httpMock: HttpTestingController;
  let storageMock;
  beforeEach(waitForAsync(() => {
    storageMock = jasmine.createSpyObj(['getItem', 'setItem']);
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule],
      providers: [{ provide: StorageService, useValue: storageMock }, PhotoStandardService],
    }).compileComponents();

    httpMock = TestBed.get(HttpTestingController);
  }));
  afterEach(() => {
    //httpMock.verify();
  });

  it('should make a http request to find out the country', () => {
    storageMock.getItem.and.returnValue(null);
    const service: PhotoStandardService = TestBed.get(PhotoStandardService);
    const req = httpMock.expectOne(`/api/location`);
    expect(req.request.method).toEqual('GET');
    req.flush({ country_code: 'US' });
    expect(service).toBeTruthy();
  });

  it('should get country code from previous value stored', () => {
    const countryCode = { country_code: 'CU' };
    const docId = 'us_passport_photo';
    storageMock.getItem.withArgs(COUNTRY_CODE).and.returnValue(countryCode);
    storageMock.getItem.withArgs(LAST_SELECTED).and.returnValue(docId);
    const service: PhotoStandardService = TestBed.get(PhotoStandardService);
    expect(service.getCountryCode()).toBe(countryCode.country_code);
    expect(service.getSelectedStandard().id).toBe(docId);
  });
});
