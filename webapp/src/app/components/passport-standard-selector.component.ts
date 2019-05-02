import {Component, Output, Input} from '@angular/core';
import {PhotoStandard} from '../model/datatypes';
import {EventEmitter} from '@angular/core';

import {HttpClient} from '@angular/common/http';

@Component({
    selector: 'app-passport-standard-selector',
    template: `
        <ng-select [allowClear]="false"
              [items]="_allStandards"
              [active]="[photoStandard]"
              (selected)="selected($event)"
              
              placeholder="No photo standard selected">
        </ng-select>
    `,
    styles: [

    ]
})
export class PassportStandardSelectorComponent {
    constructor(httpClient: HttpClient) {
        httpClient.get<PhotoStandard[]>('/assets/photo-standards.json').subscribe(
            result => {
                this._allStandards = result;
                this.photoStandard = this._allStandards.find((ps) => {
                    return ps.id === 'us_passport_photo';
                });
                this.photoStandardSelected.emit(this.photoStandard);
            },
            error => console.error(error)
        );
    }
    private _standard: PhotoStandard = new PhotoStandard('__unknown__', 'Loading ...');

    public _allStandards: PhotoStandard[];

    @Output()
    photoStandardSelected: EventEmitter<PhotoStandard> = new EventEmitter();

    @Input()
    get photoStandard() {
        return this._standard;
    }
    set photoStandard(ps: PhotoStandard) {
        this._standard = ps;
        this.photoStandardSelected.emit(this._standard);
    }

    public selected(value): void {
        this.photoStandard = this._allStandards.find((ps) => {
            return ps.id === value.id;
        });
    }
}
