import {Injectable, EventEmitter} from '@angular/core';
import {PhotoStandard} from '../model/datatypes';

import * as predefinedPhotoStandards from '../data/photo-standards.json';
import * as countryCodes from '../data/country-codes.json';

@Injectable({
    providedIn: 'root'
})
export class PhotoStandardService {
    private _predefinedStandards: PhotoStandard[];
    private _customStandards: PhotoStandard[];

    public photoStandardSelected: EventEmitter<PhotoStandard> = new EventEmitter();

    private _selectedOne: PhotoStandard;

    constructor() {
        this._predefinedStandards = (predefinedPhotoStandards as any).default;
        const defaultOne = this._predefinedStandards.find(ps => {
            return ps.id === 'us_passport_photo';
        });
        this.setSelectedStandard(defaultOne);
        // TODO: Get last selected standard from local storage
        // TODO: Fetch newly defined standards
    }

    getSelectedStandard(): PhotoStandard {
        return this._selectedOne;
    }

    setSelectedStandard(ps: PhotoStandard): void {
        this._selectedOne = ps;
        this.photoStandardSelected.emit(this._selectedOne);
    }

    addNewStandard(ps: PhotoStandard): void {}

    getAllPhotoStandards(): PhotoStandard[] {
        return this._predefinedStandards;
    }
}
