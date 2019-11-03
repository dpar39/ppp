import { Injectable, EventEmitter } from '@angular/core';
import { PrintDefinition } from '../model/datatypes.js';
import * as predefinedPrintDefinitions from '../data/print-definitions.json';

@Injectable({
  providedIn: 'root'
})
export class PrintDefinitionService {
  private _predefinedPrintDefinitions: PrintDefinition[];
  private _customPrintDefinitions: PrintDefinition[];

  public printDefinitionSelected: EventEmitter<PrintDefinition> = new EventEmitter();

  private _selectedOne: PrintDefinition;

  constructor() {
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

  addNewPrintDefinition(pd: PrintDefinition): void {}

  getAllPrintDefinitions(): PrintDefinition[] {
    return this._predefinedPrintDefinitions;
  }
}
