import { Component, OnInit, Output, Input } from '@angular/core';
import { PassportStandard, UnitType } from '../model/datatypes';
import { EventEmitter } from '@angular/core';

@Component({
    selector: 'app-passport-standard-selector',
    template: `
    <div class="dropdown" dropdown>
    <button dropdownToggle class="btn btn-primary" role="button">
      <p>{{photoStandard.name}}</p>
      <span class="caret"></span>
      <!-- <div class="form-inline " style="border: 1px solid gray; height: 2em;" ></div> -->
      <p>{{photoStandard.sizestring()}}</p>
      <!-- <div class="container-fluid">
        <div class="row form-inline">
          <input type="text" class="form-control col-5 input-sm mr-2 pull-right" placeholder="Width" OnlyNumber="true">
          <input type="text" class="form-control col-5 input-sm ml-2 pull-right" placeholder="Height" OnlyNumber="true">
        </div>
      </div> -->
    </button>
    <div *dropdownMenu class="dropdown-menu">
      <div *ngFor="let standard of _allStandards">
        <p>{{ standard.pictureWidth }}</p>
      </div>

    </div>
  </div>

  <!-- <div>
    <h5 class="card-title">{{photoIdType.name}}</h5>
    <div class="card-body">
      <p class="card-text">{{photoIdType.dimensions}}</p>
    </div>

    <div class="container-fluid">
      <div class="row form-inline">

        <input type="text" class="form-control col-5 input-sm mr-2 pull-right" placeholder="Width" OnlyNumber="true">
        <input type="text" class="form-control col-5 input-sm ml-2 pull-right" placeholder="Height" OnlyNumber="true">
      </div>
    </div>
  </div> -->
    `,
    styles: [`
    `]
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
    get photoStandard() {
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
