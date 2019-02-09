import { Component, OnInit, Output, Input } from '@angular/core';
import { PhotoStandard, UnitType } from '../model/datatypes';
import { EventEmitter } from '@angular/core';

import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-passport-standard-selector',
  template: `
    <div class="dropdown" dropdown>
      <button dropdownToggle class="btn btn-primary" role="button">
        <p>{{ photoStandard?.title }}</p>
        <span class="caret"></span>
        <!-- <div class="form-inline " style="border: 1px solid gray; height: 2em;" ></div>
      <p>{{photoStandard.sizestring()}}</p> -->
        <!-- <div class="container-fluid">
        <div class="row form-inline">
          <input type="text" class="form-control col-5 input-sm mr-2 pull-right" placeholder="Width" OnlyNumber="true">
          <input type="text" class="form-control col-5 input-sm ml-2 pull-right" placeholder="Height" OnlyNumber="true">
        </div>
      </div> -->
      </button>
      <div *dropdownMenu class="dropdown-menu">
        <div *ngFor="let standard of _allStandards">
          <p>{{ standard.title }}</p>
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
  styles: [``]
})
export class PassportStandardSelectorComponent implements OnInit {
  constructor(httpClient: HttpClient) {
    httpClient.get<PhotoStandard[]>('/assets/photo-standards.json').subscribe(
      result => {
        this._allStandards = result;

        this.photoStandard = this._allStandards[0];

        this.photoStandandardSelected.emit(this.photoStandard);
      },
      error => console.error(error)
    );
  }

  photoIdType: any;

  private _standard: PhotoStandard;

  public _allStandards: PhotoStandard[];

  @Output()
  photoStandandardSelected: EventEmitter<PhotoStandard> = new EventEmitter();

  @Input()
  get photoStandard() {
    return this._standard;
  }
  set photoStandard(ps: PhotoStandard) {
    this._standard = ps;
    this.photoStandandardSelected.emit(this._standard);
  }

  ngOnInit() {
  }
}
