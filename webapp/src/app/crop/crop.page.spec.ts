import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { IonicModule } from '@ionic/angular';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule, HttpTestingController } from '@angular/common/http/testing';
import { HttpClient, HttpClientModule } from '@angular/common/http';

import { CropPage } from './crop.page';
import { LandmarkEditorComponent } from './landmark-editor.component';
import { PhotoStandardSelectorComponent } from '../selectors/photo-standard-selector.component';
import { PrintDefinitionService } from '../services/print-definition.service';
//import { PrintDefinitionSelectorComponent } from './print-definition-selector.component';
import { BackEndService } from '../services/backend.service';
import { PhotoStandardService } from '../services/photo-standard.service';
import { StorageService } from '../services/storage.service';
import { createMockPhotoStandardService } from '../testing/mocks.spec';

describe('CropPage', () => {
  let component: CropPage;
  let fixture: ComponentFixture<CropPage>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [CropPage, LandmarkEditorComponent, PhotoStandardSelectorComponent],
      imports: [IonicModule.forRoot(), HttpClientModule, RouterTestingModule],
      providers: [
        HttpClient,
        StorageService,
        BackEndService,
        { provide: PhotoStandardService, useValue: createMockPhotoStandardService() },
        PrintDefinitionService,
      ],
    }).compileComponents();

    fixture = TestBed.createComponent(CropPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  }));

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
