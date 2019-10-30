import {Component, Output, Input} from '@angular/core';
import {PhotoStandard} from '../model/datatypes';
import {EventEmitter} from '@angular/core';

import {PhotoStandardService} from '../services/photo-standard.service';
import * as countryCodes from '../data/country-codes.json';

@Component({
    selector: 'app-photo-standard-selector',
    template: `
        <p>Select a photo standard from the list</p>

        <ion-list>
            <ion-list-header><ion-searchbar (ionInput)="filterPhotoStandard($event)"></ion-searchbar></ion-list-header>
            <ion-item *ngFor="let ps of selectableStandards">
                <span [class]="getFlagClass(ps)"></span>
                <ion-label class="ion-margin-start"> {{ ps.text }} </ion-label>
            </ion-item>
        </ion-list>

        <!--ng-select
            [items]="allPhotoStandards"
            [active]="[photoStandard]"
            (selected)="selected($event)"
            placeholder="No photo standard selected"
        >
        </ng-select-->
    `,
    styles: []
})
export class PhotoStandardSelectorComponent {
    constructor(private psService: PhotoStandardService) {
        this._allStandards = psService.getAllPhotoStandards();

        const codes = (countryCodes as any).default;
        const countries = Object.keys(codes);
        for (const country of countries) {
            const cc = codes[country];
            this.lookUp.set(country.toLowerCase().trim(), cc.toLowerCase());
        }
    }
    private lookUp = new Map<string, string>();

    private _standard: PhotoStandard = new PhotoStandard('__unknown__', 'Loading ...');

    public _allStandards: PhotoStandard[];
    public _selectableStandards: PhotoStandard[];

    public get selectableStandards() {
        return this._selectableStandards != null ? this._selectableStandards : this._allStandards;
    }

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
        this.photoStandard = this._allStandards.find(ps => {
            return ps.id === value.id;
        });
    }

    getFlagClass(ps: PhotoStandard): string {
        if (!ps.country) {
            return '';
        }
        const country = ps.country.toLowerCase();
        if (this.lookUp.has(country)) {
            return 'flag-icon flag-icon-' + this.lookUp.get(country);
        }
    }

    filterPhotoStandard(evnt) {
        const text = evnt.target.value.toLowerCase();
        if (!text) {
            this._selectableStandards = this._allStandards;
            return;
        }
        this._selectableStandards = this._allStandards.filter(ps => ps.text.toLowerCase().includes(text));
    }
}
