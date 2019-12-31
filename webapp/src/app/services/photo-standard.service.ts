import { Injectable, EventEmitter } from '@angular/core';
import { PhotoStandard } from '../model/datatypes';
import * as predefinedPhotoStandards from '../data/photo-standards.json';
import { LocalStorageService } from './local-storage.service';
import { HttpClient } from '@angular/common/http';

const COUNTRY_CODE = 'countryCode';

@Injectable({
  providedIn: 'root'
})
export class PhotoStandardService {
  private _predefinedStandards: PhotoStandard[];
  private _customStandards: PhotoStandard[];

  public photoStandardSelected: EventEmitter<PhotoStandard> = new EventEmitter();

  private _selectedOne: PhotoStandard;

  constructor(private localStorage: LocalStorageService, private http: HttpClient) {
    this._predefinedStandards = (predefinedPhotoStandards as any).default;
    const defaultOne = this._predefinedStandards.find(ps => {
      return ps.id === 'us_passport_photo';
    });
    this.setSelectedStandard(defaultOne);
    // TODO: Get last selected standard from local storage
    // TODO: Fetch newly defined standards
    this.getCountryCode();
  }

  getSelectedStandard(): PhotoStandard {
    return this._selectedOne;
  }

  setSelectedStandard(ps: PhotoStandard): void {
    this._selectedOne = ps;
    this.photoStandardSelected.emit(ps);
  }

  addNewStandard(ps: PhotoStandard): void {}

  getAllPhotoStandards(): PhotoStandard[] {
    return this._predefinedStandards;
  }

  getCountryCode() {
    const cc = this.localStorage.getItem(COUNTRY_CODE);
    if (cc) {
      return cc.country_code || cc.alpha2; // e.g. "US"
    }

    let req = 'https://api.ipgeolocationapi.com/geolocate';
    if (location.protocol === 'http:') {
      const key = 'f006103a2a5632a2e6987fde2445cf66';
      req = `http://api.ipstack.com/check?access_key=${key}`;
    }
    this.http.get(req).subscribe(res => {
      this.localStorage.setItem(COUNTRY_CODE, res);
    });
  }
}
