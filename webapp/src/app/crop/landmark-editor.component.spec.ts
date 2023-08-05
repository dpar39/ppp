import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';
import { HttpClientModule } from '@angular/common/http';

import { LandmarkEditorComponent } from './landmark-editor.component';
import { PhotoStandardService } from '../services/photo-standard.service';
import { BackEndService } from '../services/backend.service';
import { createMockPhotoStandardService } from '../testing/mocks.spec';

describe('LandmarkEditorComponent', () => {
  let component: LandmarkEditorComponent;
  let fixture: ComponentFixture<LandmarkEditorComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      imports: [HttpClientModule],
      declarations: [LandmarkEditorComponent],
      providers: [{ provide: PhotoStandardService, useValue: createMockPhotoStandardService() }, BackEndService],
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LandmarkEditorComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should be created', () => {
    expect(component).toBeTruthy();
  });
});
