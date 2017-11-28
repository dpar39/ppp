import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PassportStandardSelectorComponent } from './passport-standard-selector.component';
import { MatCardModule } from '@angular/material';



describe('PassportStandardSelectorComponent', () => {
  let component: PassportStandardSelectorComponent;
  let fixture: ComponentFixture<PassportStandardSelectorComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        PassportStandardSelectorComponent 
      ],
      imports: [
        MatCardModule
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PassportStandardSelectorComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should be created', () => {
    expect(component).toBeTruthy();
  });
});
