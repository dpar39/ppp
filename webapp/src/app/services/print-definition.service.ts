import {Injectable, EventEmitter} from '@angular/core';
import {Canvas} from '../model/datatypes.js';
import * as predefinedPrintDefinitions from '../data/print-definitions.json';

@Injectable({
    providedIn: 'root'
})
export class PrintDefinitionService {
    private _predefinedPrintDefinitions: Canvas[];
    private _customPrintDefinitions: Canvas[];

    public printDefinitionSelected: EventEmitter<Canvas> = new EventEmitter();

    private _selectedOne: Canvas;

    constructor() {
        this._predefinedPrintDefinitions = (predefinedPrintDefinitions as any).default;
        const defaultOne = this._predefinedPrintDefinitions[0];
        this.setSelectedPrintDefinition(defaultOne);
    }

    getSelectedPrintDefinition(): Canvas {
        return this._selectedOne;
    }

    setSelectedPrintDefinition(pd: Canvas): void {
        this._selectedOne = pd;
        this.printDefinitionSelected.emit(this._selectedOne);
    }

    addNewPrintDefinition(pd: Canvas): void {}

    getAllPrintDefinitions(): Canvas[] {
        return this._predefinedPrintDefinitions;
    }
}
