import { Injectable, EventEmitter } from '@angular/core';

import { convert } from '../model/geometry';
import { SvgBuilder } from '../model/svg-builder';
import { PrintDefinition, UnitType } from '../model/datatypes';

import * as predefinedPrintDefinitions from '../data/print-definitions.json';
import { PhotoDimensions } from '../model/photodimensions';
import { SafeResourceUrl } from '@angular/platform-browser';
import { SettingsService } from './settings.service';
import { StorageService } from './storage.service';

const CUSTOM_PRINT_DEFINITIONS = 'customPrintDefinitions';
@Injectable({
  providedIn: 'root',
})
export class PrintDefinitionService {
  private _predefinedPrintDefinitions: PrintDefinition[];
  private _customPrintDefinitions: PrintDefinition[] = [];

  public printDefinitionSelected: EventEmitter<PrintDefinition> = new EventEmitter();

  private _selectedOne: PrintDefinition;

  constructor(private _settings: SettingsService, private _storageService: StorageService) {
    this._customPrintDefinitions = this._storageService.getItem(CUSTOM_PRINT_DEFINITIONS) || [];
    this._predefinedPrintDefinitions = (predefinedPrintDefinitions as any).default;
    const defaultOne = this._predefinedPrintDefinitions[0];
    this.setSelectedPrintDefinition(defaultOne);
  }

  getSelectedPrintDefinition(): PrintDefinition {
    return this._selectedOne;
  }

  setSelectedPrintDefinition(pd: PrintDefinition): void {
    this._selectedOne = pd;
    this.printDefinitionSelected.emit(this._selectedOne);
  }

  getAllPrintDefinitions(): PrintDefinition[] {
    return [...this._customPrintDefinitions, ...this._predefinedPrintDefinitions];
  }

  public renderPrintSvg(ps: PhotoDimensions, imgSrc: string | SafeResourceUrl, pd: PrintDefinition = null) {
    if (!pd) {
      pd = this._selectedOne;
    }

    const dpi = Math.max(pd.resolution, ps.dpi);

    const picWidth = convert(ps.pictureWidth, ps.units, UnitType.PIXEL, dpi);
    const picHeight = convert(ps.pictureHeight, ps.units, UnitType.PIXEL, dpi);
    const printSettings = this._settings.getPrintSettings();

    let gutter = 0;
    let padding = 0;
    let numPhotoRows = 1;
    let numPhotoCols = 1;
    let printWidth = picWidth;
    let printHeight = picHeight;
    if (pd.width > 0 && pd.height > 0 && pd.units) {
      if (!(pd.gutter >= 0)) {
        gutter = convert(printSettings.gutterValue, printSettings.gutterUnits, UnitType.PIXEL, dpi);
      } else {
        gutter = convert(pd.gutter, pd.units, UnitType.PIXEL, dpi);
      }
      padding = convert(pd.padding, pd.units, UnitType.PIXEL, dpi);
      printWidth = convert(pd.width + 2 * pd.padding, pd.units, UnitType.PIXEL, dpi);
      printHeight = convert(pd.height + 2 * pd.padding, pd.units, UnitType.PIXEL, dpi);
      numPhotoRows = Math.floor(printHeight / (picHeight + gutter));
      numPhotoCols = Math.floor(printWidth / (picWidth + gutter));
    }

    const dx = picWidth + gutter;
    const dy = picHeight + gutter;
    let offsetX = padding;
    let offsetY = padding;

    const centerAlign = printSettings.centerAlign;
    if (centerAlign) {
      offsetX = (printWidth - numPhotoCols * dx + gutter) / 2;
      offsetY = (printHeight - numPhotoRows * dy + gutter) / 2;
    }

    const paperColor = pd.paperColor ? pd.paperColor : printSettings.paperColor;
    const builder = new SvgBuilder(printWidth, printHeight, paperColor);

    for (let row = 0; row < numPhotoRows; ++row) {
      for (let col = 0; col < numPhotoCols; ++col) {
        const x = offsetX + col * dx;
        const y = offsetY + row * dy;
        builder.addImage(imgSrc, x, y, picWidth, picHeight);
      }
    }

    return builder.getSvg();
  }

  public getMaximumNumberOfTiles(photoDim: PhotoDimensions, pd: PrintDefinition) {
    const dpi = Math.max(pd.resolution, photoDim.dpi);

    const picWidth = convert(photoDim.pictureWidth, photoDim.units, UnitType.PIXEL, dpi);
    const picHeight = convert(photoDim.pictureHeight, photoDim.units, UnitType.PIXEL, dpi);

    const gutter = convert(pd.gutter, pd.units, UnitType.PIXEL, dpi);
    const printWidth = convert(pd.width + 2 * pd.padding, pd.units, UnitType.PIXEL, dpi);
    const printHeight = convert(pd.width + 2 * pd.padding, pd.units, UnitType.PIXEL, dpi);

    const numPhotoRows = Math.floor(printHeight / (picHeight + gutter));
    const numPhotoCols = Math.floor(printWidth / (picWidth + gutter));
    return numPhotoRows * numPhotoCols;
  }

  update(pd: PrintDefinition) {
    if (!pd) {
      return;
    }

    const oldPd = this._customPrintDefinitions.find((a) => a.title == pd.title);
    if (!oldPd) {
      this._customPrintDefinitions.push(pd);
      this._selectedOne = pd;
    } else {
      Object.assign(oldPd, pd);
      this._selectedOne = oldPd;
    }

    this._storageService.setItem(CUSTOM_PRINT_DEFINITIONS, this._customPrintDefinitions);
  }

  delete(pd: PrintDefinition) {
    const index = this._customPrintDefinitions.findIndex((a) => a.title == pd.title);
    if (index > -1) {
      this._customPrintDefinitions.splice(index, 1);
      this._storageService.setItem(CUSTOM_PRINT_DEFINITIONS, this._customPrintDefinitions);
    }
  }
}
