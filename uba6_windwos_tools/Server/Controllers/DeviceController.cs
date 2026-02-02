using Microsoft.AspNetCore.Mvc;
using Server.UBA_Device;
using System.Collections.Generic;
using System.Reflection.Metadata.Ecma335;

namespace Server.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class DeviceController : ControllerBase {
        private readonly ILogger<DeviceController> _logger;

        private static readonly List<DeviceDTO> devices = new List<DeviceDTO>() {
            new DeviceDTO(){Id = 1, Name ="UBA 1",Channel =DeviceDTO.CHANNEL.A}
        };


        public DeviceController(ILogger<DeviceController> logger) {
            _logger = logger;
        }

        [HttpGet("Total")]
        public ActionResult<TotalDeviceDTO> GetTotalDevice() {
            //TODO: Find all Devices using the UBA Driver  
            return new TotalDeviceDTO(30, 30, 30);
        }

        [HttpGet("Devies")]
        public IEnumerable<DeviceDTO> GetlDevices() {
            return devices.ToArray();
        }

        [HttpGet("{ID}")]
        public ActionResult<DeviceDTO> Get(UInt32 ID) {
            var device = devices.FirstOrDefault(i => i.Id == ID);
            if (device == null) {
                return NotFound();
            } else {
                return device;
            }
        }

        [HttpPost]
        public ActionResult<DeviceBaseDTO> Post(DeviceBaseDTO newDeviceBase) {
            if (devices.Any(device => device == newDeviceBase)) {
                return Conflict();
            } else {
                DeviceDTO newDevice = DeviceDTO.CreateNewDevice(newDeviceBase);
                devices.Add(newDevice);
                return CreatedAtAction(nameof(Get), new { ID = newDevice.Id }, newDevice);
            }
        }

        [HttpPut]
        public ActionResult<DeviceDTO> Put(DeviceBaseDTO updateDevice) {
            DeviceDTO device = devices.FirstOrDefault(findDevice => findDevice == updateDevice);
            if (devices == null) {
                return NotFound();
            } else {
                device.Update(updateDevice);
                return CreatedAtAction(nameof(Get), new { ID = updateDevice.Port }, updateDevice);
            }
        }


        [HttpDelete("{ID}")]
        public ActionResult Delete(UInt32 ID) {
            var device = devices.FirstOrDefault(i => i.Id == ID);
            if (device == null) {
                return NotFound();
            }
            devices.Remove(device);
            return NoContent();
        } 
    }
}
