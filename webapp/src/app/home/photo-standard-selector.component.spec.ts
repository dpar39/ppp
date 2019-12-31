import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PhotoStandardSelectorComponent } from './photo-standard-selector.component';
import { HttpClientModule } from '@angular/common/http';
import { IonicModule } from '@ionic/angular';
import { RouterTestingModule } from '@angular/router/testing';
import { PhotoStandardService } from '../services/photo-standard.service';

describe('PhotoStandardSelectorComponent', () => {
  let component: PhotoStandardSelectorComponent;
  let fixture: ComponentFixture<PhotoStandardSelectorComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [PhotoStandardSelectorComponent],
      imports: [HttpClientModule, IonicModule, RouterTestingModule],
      providers: [PhotoStandardService]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PhotoStandardSelectorComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should be created', () => {
    expect(component).toBeTruthy();
  });
});
