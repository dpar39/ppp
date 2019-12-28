import { async, TestBed } from '@angular/core/testing';

import { PhotoStandardService } from './photo-standard.service';
import { HttpClient, HttpClientModule } from '@angular/common/http';
import { LocalStorageService } from './local-storage.service';

describe('PhotoStandardService', () => {

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      imports: [HttpClientModule],
      providers: [HttpClient, LocalStorageService, PhotoStandardService]
    }).compileComponents();
  }));

  it('should be created', () => {
    const service: PhotoStandardService = TestBed.get(PhotoStandardService);
    expect(service).toBeTruthy();
  });
});
