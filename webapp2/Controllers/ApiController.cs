using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Net.Http.Headers;

namespace webapi.Controllers
{
    [Route("api")]
    public class ApiController : Controller
    {
        private readonly IHostingEnvironment _hostingEnv;

        public ApiController(IHostingEnvironment env)
        {
            _hostingEnv = env;
        }

        // GET api/values
        [HttpGet]
        public IEnumerable<string> Get()
        {
            return new string[] { "value1", "value2" };
        }


        // POST api/upload
        [HttpPost("upload")]
        public IActionResult Upload()
        {
            var files = Request.Form?.Files;
            if (files == null || files.Count != 1)
            {
                return BadRequest();
            }
            var imgFile = files.Single();
            SaveImageToFileDisk(imgFile);

            return Json(new { imgKey = "key" });
        }

        [HttpGet("test")]
        public IActionResult Test()
        {
            return Json(new { status = "it works" });
        }

        private void SaveImageToFileDisk(IFormFile file)
        {
            var uploadingDirectory = Path.Combine(_hostingEnv.ContentRootPath, "uploads");
            if (!Directory.Exists(uploadingDirectory))
            {
                Directory.CreateDirectory(uploadingDirectory);
            }
            var imgFilePath = Path.Combine(uploadingDirectory, file.FileName);
            using (FileStream fs  = System.IO.File.Create(imgFilePath))
            {
                file.CopyTo(fs);
            }
        }
    }
}
