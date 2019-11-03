import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PhotoStandardSelectorComponent } from './photo-standard-selector.component';
import { HttpClientModule } from '@angular/common/http';
import { IonicModule } from '@ionic/angular';

describe('PhotoStandardSelectorComponent', () => {
  let component: PhotoStandardSelectorComponent;
  let fixture: ComponentFixture<PhotoStandardSelectorComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [PhotoStandardSelectorComponent],
      imports: [HttpClientModule, IonicModule]
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
