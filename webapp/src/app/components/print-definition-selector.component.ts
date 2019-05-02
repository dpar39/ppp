import {Component, Output, Input, EventEmitter} from '@angular/core';
import {HttpClient} from '@angular/common/http';

import {Canvas} from '../model/datatypes';

@Component({
    selector: 'app-print-definition-selector',
    template: `
        <p>Select a print definition from the list</p>
        <ng-select
            [allowClear]="false"
            [items]="_allPrintDefinitions"
            [active]="[selectedPrintDefinition]"
            (selected)="selected($event)"
            placeholder="No print definition selected"
        >
        </ng-select>
    `,
    styles: []
})
export class PrintDefinitionSelectorComponent {
    constructor(httpClient: HttpClient) {
        httpClient.get<Canvas[]>('/assets/print-definitions.json').subscribe(
            result => {
                for (const pd of result) {
                    const units = pd.units === 'inch' ? '″' : pd.units;
                    pd.text = pd.id = `${pd.height} x ${pd.width}${units} @ ${pd.resolution}dpi`;
                }
                this._allPrintDefinitions = result;
                this.selectedPrintDefinition = this._allPrintDefinitions[0];
                this.printDefinitionSelected.emit(this.selectedPrintDefinition);
            },
            error => console.error(error)
        );
    }

    private _selectedPrintDefinition: Canvas = new Canvas('__unknown__', 'Loading ...');
    public _allPrintDefinitions: Canvas[];

    @Output()
    printDefinitionSelected: EventEmitter<Canvas> = new EventEmitter();

    @Input()
    get selectedPrintDefinition() {
        return this._selectedPrintDefinition;
    }
    set selectedPrintDefinition(ps: Canvas) {
        this._selectedPrintDefinition = ps;
        this.printDefinitionSelected.emit(this._selectedPrintDefinition);
    }

    public selected(value): void {
        this.selectedPrintDefinition = this._allPrintDefinitions.find(ps => {
            return ps.id === value.id;
        });
    }
}
