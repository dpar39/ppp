import { Component, OnInit, Output, Input } from '@angular/core';
import { PassportStandard, UnitType } from '../model/datatypes';
import { EventEmitter } from '@angular/core';
import { THIS_EXPR } from '@angular/compiler/src/output/output_ast';

@Component({
    selector: 'app-passport-standard-selector',
    templateUrl: './passport-standard-selector.component.html',
    styleUrls: ['./passport-standard-selector.component.css']
})
export class PassportStandardSelectorComponent implements OnInit {

    constructor() {
        this._allStandards = [
            new PassportStandard(35, 45, 34, UnitType.mm, 'US passport'),
            new PassportStandard(2, 2, 1.1875, UnitType.inch, 'Australian Passport')
        ];
        this._standard = this._allStandards[0];
    }


    photoIdType: any;

    private _standard: PassportStandard;

    public _allStandards: PassportStandard[];

    @Output()
    selectedStandard: EventEmitter<PassportStandard> = new EventEmitter();

    @Input()
    get counter() {
      return this._standard;
    }
    set photoStandard(ps: PassportStandard) {
        this._standard = ps;
        this.selectedStandard.emit(this._standard);
    }

    ngOnInit() {

        this.photoIdType = {
            name: 'US Passport',
            dimensions: '2\" x 2"'
        };
    }


}
