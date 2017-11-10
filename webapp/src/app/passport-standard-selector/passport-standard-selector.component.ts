import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-passport-standard-selector',
  templateUrl: './passport-standard-selector.component.html',
  styleUrls: ['./passport-standard-selector.component.css']
})
export class PassportStandardSelectorComponent implements OnInit {

  constructor() { }

  idType: any;

  ngOnInit() {

    this.idType = {
      name: 'US Passport',
      dimensions: '2\" x 2"'
    };
  }

}
