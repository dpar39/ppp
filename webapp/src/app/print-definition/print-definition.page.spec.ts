import { CUSTOM_ELEMENTS_SCHEMA } from '@angular/core';
import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { Location } from '@angular/common';
import { HttpClientModule, HttpClient } from '@angular/common/http';

import { PrintDefinitionPage } from './print-definition.page';
import { PrintDefinitionService } from '../services/print-definition.service';
import { IonicModule } from '@ionic/angular';
import { PhotoStandardService } from '../services/photo-standard.service';
import { ColorPickerModule } from 'ngx-color-picker';

describe('PrintDefinitionPage', () => {
  let component: PrintDefinitionPage;
  let fixture: ComponentFixture<PrintDefinitionPage>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [PrintDefinitionPage],
      providers: [PrintDefinitionService, PhotoStandardService, Location, HttpClient, ColorPickerModule],
      imports: [RouterTestingModule, HttpClientModule, IonicModule.forRoot()],
      schemas: [CUSTOM_ELEMENTS_SCHEMA],
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PrintDefinitionPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
