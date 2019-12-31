import { CUSTOM_ELEMENTS_SCHEMA } from '@angular/core';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { Location } from '@angular/common';
import { HttpClientModule, HttpClient } from '@angular/common/http';

import { PhotoStandardPage } from './photo-standard.page';
import { PhotoStandardService } from '../services/photo-standard.service';

describe('PhotoStandardPage', () => {
  let component: PhotoStandardPage;
  let fixture: ComponentFixture<PhotoStandardPage>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PhotoStandardPage ],
      providers: [PhotoStandardService, Location, HttpClient],
      imports: [RouterTestingModule, HttpClientModule],
      schemas: [CUSTOM_ELEMENTS_SCHEMA],
    })
    .compileComponents();
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
