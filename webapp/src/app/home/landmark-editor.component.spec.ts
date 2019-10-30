import {async, ComponentFixture, TestBed} from '@angular/core/testing';

import {LandmarkEditorComponent} from './landmark-editor.component';

describe('LandmarkEditorComponent', () => {
    let component: LandmarkEditorComponent;
    let fixture: ComponentFixture<LandmarkEditorComponent>;

    beforeEach(async(() => {
        TestBed.configureTestingModule({
            declarations: [LandmarkEditorComponent]
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
