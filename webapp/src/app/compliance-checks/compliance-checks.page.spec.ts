import { CUSTOM_ELEMENTS_SCHEMA } from '@angular/core';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ComplianceChecksPage } from './compliance-checks.page';

describe('ComplianceChecksPage', () => {
  let component: ComplianceChecksPage;
  let fixture: ComponentFixture<ComplianceChecksPage>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ComplianceChecksPage ],
      schemas: [CUSTOM_ELEMENTS_SCHEMA],
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ComplianceChecksPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
