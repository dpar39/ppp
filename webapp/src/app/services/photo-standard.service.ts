import { Injectable, EventEmitter } from '@angular/core';
import { HttpClient } from '@angular/common/http';

import { PhotoStandard } from '../model/datatypes';
import * as predefinedPhotoStandards from '../data/photo-standards.json';
import { StorageService } from './storage.service';
import { getCroppingRectangle, PhotoDimensions } from '../model/photodimensions';
import { Point, getAffineTransform, convert } from '../model/geometry';

const COUNTRY_CODE = 'countryCode';
const LAST_SELECTED = 'lastSelected';

@Injectable({
  providedIn: 'root',
})
export class PhotoStandardService {
  private _predefinedStandards: PhotoStandard[];
  private _customStandards: PhotoStandard[];

  public photoStandardSelected: EventEmitter<PhotoStandard> = new EventEmitter();
  public croppedImageChanged: EventEmitter<string> = new EventEmitter();

  private _selectedOne: PhotoStandard;
  private _croppedImageBlobUrl = '';
  private _croppedImageDataUrl = '';

  constructor(private storage: StorageService, private http: HttpClient) {
    this._predefinedStandards = (predefinedPhotoStandards as any).default;
    this._predefinedStandards.sort((a, b) => (a.docType === 'Passport' ? -1 : 1) - (b.docType === 'Passport' ? -1 : 1));
    const lastPsId = this.storage.getItem(LAST_SELECTED) || 'us_passport_photo';
    const defaultOne = this._predefinedStandards.find((ps) => {
      return ps.id === lastPsId;
    });
    this.setSelectedStandard(defaultOne);
    // TODO: Get last selected standard from local storage
    // TODO: Fetch newly defined standards
    this.getCountryCode();

    if (!this._croppedImageBlobUrl) {
      const image = new Image();
      image.src = 'assets/face.png';
      image.onload = () => {
        this.createCroppedImage(image, new Point(289, 131), new Point(271, 395));
      };
    }
  }

  getSelectedStandard(): PhotoStandard {
    return this._selectedOne;
  }

  setSelectedStandard(ps: PhotoStandard): void {
    this._selectedOne = ps;
    this.storage.setItem(LAST_SELECTED, ps.id);
    this.photoStandardSelected.emit(ps);
  }

  addNewStandard(ps: PhotoStandard): void {}

  getAllPhotoStandards(): PhotoStandard[] {
    return this._predefinedStandards;
  }

  getCountryCode(): string {
    const cc = this.storage.getItem(COUNTRY_CODE);
    if (cc) {
      return cc.country_code || cc.alpha2; // e.g. "US"
    }
    this.http.get('/api/location').subscribe(
      (res) => {
        this.storage.setItem(COUNTRY_CODE, res);
      },
      (err) => {
        this.storage.setItem(COUNTRY_CODE, { country_code: 'US' });
      }
    );
    return cc;
  }

  public createCroppedImage(
    image: CanvasImageSource,
    crownPoint: Point,
    chinPoint: Point,
    p: PhotoDimensions = null
  ): void {
    if (!crownPoint || !chinPoint || crownPoint.equals(chinPoint) || !image) {
      return;
    }
    if (p == null) {
      p = this._selectedOne.dimensions;
    }
    const rotRect = getCroppingRectangle(p, crownPoint, chinPoint);
    //const width = rotRect.length2;
    //const height = rotRect.length1;

    const width = convert(p.pictureWidth, p.units, 'pixel', p.dpi);
    const height = convert(p.pictureHeight, p.units, 'pixel', p.dpi);

    const canvas = document.createElement('canvas');
    canvas.width = width;
    canvas.height = height;
    const ctx = canvas.getContext('2d');
    const fromPoints = rotRect.corners();
    const toPoints = [new Point(0, height), new Point(width, height), new Point(width, 0), new Point(0, 0)];
    const [a, b, c, d, e, f] = getAffineTransform(fromPoints, toPoints);
    ctx.save();
    ctx.transform(a, b, c, d, e, f);
    ctx.drawImage(image, 0, 0);
    ctx.restore();
    canvas.toBlob((blob) => {
      const url = URL.createObjectURL(blob);
      if (this._croppedImageBlobUrl) {
        URL.revokeObjectURL(this._croppedImageBlobUrl);
      }
      this._croppedImageBlobUrl = url;
      this.croppedImageChanged.emit(this._croppedImageBlobUrl);
    });

    this._croppedImageDataUrl = canvas.toDataURL();
    // this.croppedImageChanged.emit(this._croppedImageDataUrl);
  }

  getCroppedImageBlobUrl(): string {
    return this._croppedImageBlobUrl;
  }

  getCroppedImageDataUrl(): string {
    return this._croppedImageDataUrl;
  }
}
