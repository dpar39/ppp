import { TestBed } from '@angular/core/testing';

import { PhotoStandardService } from './photo-standard.service';

describe('PhotoStandardService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: PhotoStandardService = TestBed.get(PhotoStandardService);
    expect(service).toBeTruthy();
  });
});
