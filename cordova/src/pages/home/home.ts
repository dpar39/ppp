import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { AfterViewInit} from '@angular/core';

declare var cpp: any;

@Component({
  selector: 'page-home',
  templateUrl: 'home.html'
})
export class HomePage implements AfterViewInit {
  constructor(public navCtrl: NavController) {
  }
  cppProp: string = ".....";

  ngAfterViewInit() {
    this.cppProp = "xxxxxxx";
    try{
      if (cpp) {
        cpp.TestCpp.staticTestMethod(ret => {
          this.cppProp = ret;
        });
      }
    }
    catch (Exception) {
    }
  }
}
