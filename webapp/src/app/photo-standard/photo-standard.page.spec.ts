import { CUSTOM_ELEMENTS_SCHEMA } from '@angular/core';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { PhotoStandardPage } from './photo-standard.page';
import { PhotoStandardService } from '../services/photo-standard.service';
import { createMockPhotoStandardService } from '../testing/mocks.spec';

describe('PhotoStandardPage', () => {
  let component: PhotoStandardPage;
  let fixture: ComponentFixture<PhotoStandardPage>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [PhotoStandardPage],
      providers: [{ provide: PhotoStandardService, useValue: createMockPhotoStandardService() }],
      imports: [RouterTestingModule],
      schemas: [CUSTOM_ELEMENTS_SCHEMA],
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PhotoStandardPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
