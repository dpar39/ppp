import { TestBed, inject } from '@angular/core/testing';

import { BackEndService } from './back-end.service';

describe('BackEndServiceService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [BackEndService]
    });
  });

  it('should be created', inject([BackEndService], (service: BackEndService) => {
    expect(service).toBeTruthy();
  }));
});
