// import { Component } from '@angular/core';
// import { PrintDefinition } from '../model/datatypes';
// import { PrintDefinitionService } from '../services/print-definition.service';
// import { PhotoStandardService } from '../services/photo-standard.service';
// import { PhotoDimensions } from '../model/photodimensions';
// import { renderAsPng } from '../model/svg-builder';

// @Component({
//   selector: 'app-print-definition-selector',
//   template: `
//     <ion-card routerLink="/prints" class="margin-vertical">
//       <table>
//         <tr style="width: 100%">
//           <td style="width: 99%">
//             <ion-card-header class="half-padding">
//               Selected print output
//             </ion-card-header>
//             <ion-card-content class="ion-no-padding">
//               <ion-grid>
//                 <ion-row>
//                   <ion-col>
//                     <ion-label class="ion-margin-start" color="secondary">
//                       {{ getStringRepr(printDefinition) }}
//                     </ion-label>
//                   </ion-col> </ion-row
//                 ><ion-row>
//                   <ion-col style="text-align:right">
//                     <ion-button fill="outline" class="round" [disabled]="!svgElement" (click)="download($event)">
//                       <ion-icon name="download"></ion-icon>
//                     </ion-button>
//                     <ion-button
//                       fill="outline"
//                       *ngIf="canShare()"
//                       class="round"
//                       [disabled]="!svgElement"
//                       (click)="share($event)"
//                     >
//                       <ion-icon name="share-social-outline"></ion-icon>
//                     </ion-button>
//                     <ion-button
//                       fill="outline"
//                       *ngIf="false"
//                       class="round"
//                       [disabled]="!svgElement"
//                       (click)="print($event)"
//                     >
//                       <ion-icon name="print"></ion-icon>
//                     </ion-button>
//                   </ion-col>
//                 </ion-row>
//               </ion-grid>
//             </ion-card-content>
//           </td>
//           <td id="thumbnail-container">
//             <div id="thumbnail"></div>
//           </td>
//         </tr>
//       </table>
//     </ion-card>
//     <!--ion-list *ngIf="!collapsed">
//       <ion-list-header class="ion-no-padding">
//         <ion-searchbar
//           placeholder="Search Print Definitions"
//           (ionInput)="filterPrintDefinition($event)"
//         ></ion-searchbar>
//       </ion-list-header>
//       <ion-item *ngFor="let pd of selectablePrintDefinitions" (click)="setSelected(pd)">
//         <ion-label class="ion-margin-start"> {{ getStringRepr(pd) }} </ion-label>
//         <ion-button color="success" icon-only><ion-icon name="create"></ion-icon></ion-button>
//       </ion-item>
//     </ion-list-->
//   `,
//   styles: [
//     `
//       .half-padding {
//         padding: 6px 16px !important;
//       }
//       .round {
//         --border-radius: 10px;
//         --vertical-align: middle;
//         --padding-start: 5px;
//         --padding-end: 5px;
//       }
//       #thumbnail-container {
//         padding: 5px;
//       }
//     `,
//   ],
// })
// export class PrintDefinitionSelectorComponent {
//   public svgElement: SVGElement;
//   public printDefinition: PrintDefinition = new PrintDefinition('__unknown__', 'Loading ...');

//   private _allPrintDefinitions: PrintDefinition[];
//   private _selectablePrintDefinitions: PrintDefinition[];
//   private _photoDimensions: PhotoDimensions;

//   private _croppedImageUrl: string;

//   private THUMBNAIL_HEIGHT = 110;

//   constructor(private pdService: PrintDefinitionService, private psService: PhotoStandardService) {
//     this._allPrintDefinitions = pdService.getAllPrintDefinitions();
//     this.printDefinition = pdService.getSelectedPrintDefinition();

//     pdService.printDefinitionSelected.subscribe((pd) => {
//       if (pd !== this.printDefinition) {
//         this.printDefinition = pd;
//       }
//       this.updateView();
//     });

//     psService.photoStandardSelected.subscribe((ps) => {
//       this._photoDimensions = ps.dimensions;
//     });
//     this._photoDimensions = psService.getSelectedStandard().dimensions;

//     psService.croppedImageChanged.subscribe((croppedImageUrl) => {
//       this._croppedImageUrl = croppedImageUrl;
//       this.updateView();
//     });
//   }

//   private _collapsed = true;
//   public get collapsed() {
//     return this._collapsed;
//   }
//   public set collapsed(value: boolean) {
//     if (value === this._collapsed) {
//       return;
//     }
//     this._collapsed = value;
//     this._selectablePrintDefinitions = this._allPrintDefinitions;
//   }

//   public get selectablePrintDefinitions() {
//     return this._selectablePrintDefinitions != null ? this._selectablePrintDefinitions : this._allPrintDefinitions;
//   }

//   public setSelected(pd: PrintDefinition): void {
//     this.pdService.setSelectedPrintDefinition(pd);
//     this.collapsed = true;
//   }

//   updateView() {
//     const container = document.getElementById('thumbnail');
//     if (!container) {
//       return;
//     }
//     while (container.firstChild) {
//       container.removeChild(container.firstChild);
//     }

//     this.svgElement = this.pdService.renderPrintSvg(this._photoDimensions, this._croppedImageUrl);
//     const svgThumbnail = this.svgElement.cloneNode(true) as SVGSVGElement;
//     const w = +svgThumbnail.getAttribute('width');
//     const h = +svgThumbnail.getAttribute('height');
//     const ww = (this.THUMBNAIL_HEIGHT * w) / h;
//     const hh = this.THUMBNAIL_HEIGHT;
//     svgThumbnail.setAttribute('width', `${ww}px`);
//     svgThumbnail.setAttribute('height', `${hh}px`);
//     container.appendChild(svgThumbnail);
//     container.style.height = `${hh}px`;
//   }

//   filterPrintDefinition(event) {
//     const text = event.target.value.toLowerCase();
//     if (!text) {
//       this._selectablePrintDefinitions = this._allPrintDefinitions;
//       return;
//     }
//     this._selectablePrintDefinitions = this._allPrintDefinitions.filter((ps) =>
//       this.getStringRepr(ps).toLowerCase().includes(text)
//     );
//   }

//   getStringRepr(pd: PrintDefinition): string {
//     if (pd.text) {
//       return pd.text;
//     }
//     const units = pd.units === 'inch' ? '″' : pd.units;
//     return `${pd.height} x ${pd.width}${units} [${pd.resolution}dpi]`;
//   }

//   download(event) {
//     event.preventDefault();
//     event.stopPropagation();

//     this.prepareOutput((file) => {
//       const a = document.createElement('a');
//       const url = URL.createObjectURL(file);
//       a.setAttribute('href', url);
//       a.setAttribute('download', file.name);
//       document.body.appendChild(a);
//       a.click();
//       URL.revokeObjectURL(url);
//       document.body.removeChild(a);
//     });
//   }

//   prepareOutput(callback: (file: File) => void) {
//     const pd = this.pdService.getSelectedPrintDefinition();

//     const blobUrl = this.psService.getCroppedImageBlobUrl();
//     const dataUrl = this.psService.getCroppedImageDataUrl();
//     const dpi = this.psService.getSelectedStandard().dimensions.dpi;

//     const cb = (blob: Blob) => {
//       const filename =
//         pd.height > 0 && pd.width > 0 && pd.units
//           ? `${pd.height}x${pd.width}${pd.units}-print.png`
//           : pd.text.toLocaleLowerCase().replace(' ', '-') + '.png';
//       let file: any = blob;
//       file.lastModifiedDate = new Date();
//       file.name = filename;
//       callback(<File>file);
//     };
//     renderAsPng(this.svgElement as SVGSVGElement, dpi, (blob) => cb(blob), blobUrl, dataUrl);
//   }

//   share(event) {
//     event.preventDefault();
//     event.stopPropagation();

//     const navigator = window.navigator as any;
//     if (this.canShare()) {
//       this.prepareOutput((file) => {
//         if (navigator.canShare({ files: [file] })) {
//           let f = file;
//           navigator
//             .share({
//               files: [file],
//               title: file.name,
//               text: 'Print Output',
//             })
//             .then(() => console.log('Share was successful.'))
//             .catch((error) => console.log('Sharing failed', error));
//         } else {
//           console.log(`Your system doesn't support sharing files.`);
//         }
//       });
//     }
//   }

//   canShare() {
//     return !!(window.navigator as any).canShare;
//   }

//   print(event) {
//     event.preventDefault();
//     event.stopPropagation();
//   }
// }
