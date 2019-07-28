import {async, ComponentFixture, TestBed} from '@angular/core/testing';

import {PassportStandardSelectorComponent as PhotoStandardSelectorComponent} from './photo-standard-selector.component';
import {HttpClientModule} from '@angular/common/http';
import {SelectModule} from 'ng2-select/ng2-select';

describe('PhotoStandardSelectorComponent', () => {
    let component: PhotoStandardSelectorComponent;
    let fixture: ComponentFixture<PhotoStandardSelectorComponent>;

    beforeEach(async(() => {
        TestBed.configureTestingModule({
            declarations: [PhotoStandardSelectorComponent],
            imports: [HttpClientModule, SelectModule]
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
