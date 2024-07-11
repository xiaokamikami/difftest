package difftest.json

import chisel3._
import chisel3.util._
import difftest._
import difftest.gateway._
import difftest.json._
import ujson._
import scala.collection.mutable._
import scala.collection.mutable

object DiffJsonTop {
  private val instances = ListBuffer.empty[DifftestBundle]

  // DifftestModule(new DiffInstrCommit(param1), delay = param2, dontCare = param3)),
  // val result = DataCollector.query("DiffArchFpRegState", "delay", "exception_count")
  // println(result) // 预期输出：Map("delay" -> Some(2), "exception_count" -> Some(2))

  // // 提取值到变量
  // val delay: Option[Int] = result.get("delay").flatMap(_.asInstanceOf[Option[Int]])
  // val exceptionCount: Option[Int] = result.get("exception_count").flatMap(_.asInstanceOf[Option[Int]])

  def creat_top(config: GatewayConfig): GatewayEndpoint = {
    val json = DataCollector.readFromFile("difftest/diff_class_info.json")
    instantiateModules(json)
    println("creat endpoint")
    val endpoint = Module(new GatewayEndpoint(instances.toSeq, config))
    println("connet endpoint step")
    val step = endpoint.step
    endpoint
  }

  def instantiateModules(jsonData: mutable.Map[String, mutable.Map[String, Any]]): Unit = {
    ModuleRegistry.initialize()
    jsonData.foreach { case (className, params) =>
      println(s"Instantiating module: $className with params: $params")
      val delay = params.get("delay").map(_.asInstanceOf[Double].toInt).getOrElse(0)
      val exceptionCount = params.get("exception_count").map(_.asInstanceOf[Double].toInt).getOrElse(0)
      val numRegs = params.get("numRegs").map(_.asInstanceOf[Double].toInt).getOrElse(0)
      val constructor = ModuleRegistry.getModule(className).getOrElse(throw new Exception(s"Unknown class name: $className"))
      (1 to exceptionCount).foreach { _ =>
        val moduleInstance = constructor(Some(numRegs), delay, true).asInstanceOf[DifftestBundle]
        // Automatically set all IO to 1
        // setAllIOToDefault(moduleInstance.io)
        val bundle = WireInit(0.U.asTypeOf(moduleInstance))
        // val packed = WireInit(bundle.asUInt)
        // DifftestWiring.addSource(packed, s"gateway_${instances.length}")
        instances += moduleInstance
      }
    }
  }
  def setAllIOToDefault(io: DifftestBundle): Unit = {
    io match {
      case bundle: Bundle =>
        bundle.elements.foreach {
          case (_, boolData: chisel3.Bool) => boolData := true.B
          case (_, uintData: UInt) => uintData := Fill(uintData.getWidth, true.B)
          case (_, sintData: SInt) => sintData := Fill(sintData.getWidth, true.B).asSInt
          case (_, vecData: Vec[_]) => vecData.foreach(setAllIOPortsToDefault)
          case (_, _) => // Handle other types if needed
        }
      case _ =>
    }
  }
  def setAllIOPortsToDefault(data: Data): Unit = {
    data match {
      case boolData: chisel3.Bool => boolData := true.B
      case uintData: UInt => uintData := Fill(uintData.getWidth, true.B)
      case sintData: SInt => sintData := Fill(sintData.getWidth, true.B).asSInt
      case bundleData: Bundle => setAllIOPortsToDefault(bundleData)
      case vecData: Vec[_] => vecData.foreach(setAllIOPortsToDefault)
      case _ => // Handle other types if needed
    }
  }
}
/*
object ModuleRegistry {
  type ModuleConstructor = (Option[Int], Int, Boolean) => DifftestModule[_ <: DifftestBundle]
  //private val moduleMap: mutable.Map[String, ModuleConstructor] = mutable.Map()
  // 构造函数映射
  private val moduleMap: Map[String, ModuleConstructor] = Map(
    "DiffInstrCommit" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffInstrCommit(param1.getOrElse(0)), delay = param2, dontCare = param3)
    ),
    "DiffArchEvent" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffArchEvent, delay = param2, dontCare = param3)
    ),
    "DiffL1TLBEvent" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffL1TLBEvent)
    ),
    "DiffL2TLBEvent" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffL2TLBEvent)
    ),
    "DiffLrScEvent" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffLrScEvent)
    ),
    "DiffHCSRState" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffHCSRState)
    ),
    "DiffAtomicEvent" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffAtomicEvent)
    ),
    "DiffLoadEvent" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffLoadEvent, delay = param2)
    ),
    "DiffArchIntRegState" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffArchIntRegState, delay = param2)
    ),
    "DiffArchFpRegState" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffArchFpRegState, delay = param2)
    ),
    "DiffArchVecRegState" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffArchVecRegState, delay = param2)
    ),
    "DiffIntWriteback" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffIntWriteback(param1.getOrElse(0)))
    ),
    "DiffFpWriteback" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffFpWriteback(param1.getOrElse(0)))
    ),
    "DiffVecWriteback" -> ((param1: Option[Int], param2:Int, param3: Boolean) =>
      DifftestModule(new DiffVecWriteback(param1.getOrElse(0)))
    ),
  )
 
  def getModule(className: String): Option[ModuleConstructor] = {
    println(s"Looking for className: $className")
    val result = moduleMap.get(className)
    if (result.isEmpty) {
      println(s"Warning: ClassName $className not found in moduleMap.")
      println(s"Current keys in moduleMap: ${moduleMap.keys.mkString(", ")}")
    }
    result
  }
}
*/
object ModuleRegistry {

  type ModuleConstructor = (Option[Int], Int, Boolean) => Any

  private val moduleMap: mutable.Map[String, ModuleConstructor] = mutable.Map()

  // 注册模块
  def register(className: String, constructor: ModuleConstructor): Unit = {
    moduleMap += (className -> constructor)
  }

  // 获取模块
  def getModule(className: String): Option[ModuleConstructor] = {
    moduleMap.get(className)
  }

  // 初始化模块注册表
  def initialize(): Unit = {
    register("DiffInstrCommit", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffInstrCommit(param1.getOrElse(0)), delay = param2, dontCare = param3)
    )
    register("DiffArchEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchEvent, delay = param2, dontCare = param3)
    )
    register("DiffL1TLBEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffL1TLBEvent)
    )
    register("DiffL2TLBEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffL2TLBEvent)
    )
    register("DiffLrScEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffLrScEvent)
    )
    register("DiffDebugMode", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffDebugMode)
    )
    register("DiffCSRState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffCSRState)
    )
    register("DiffHCSRState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffHCSRState)
    )
    register("DiffVecCSRState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffVecCSRState)
    )
    register("DiffAtomicEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffAtomicEvent)
    )
    register("DiffTrapEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffIntWriteback, dontCare = param3)
    )
    register("DiffRefillEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffRefillEvent, dontCare = param3)
    )
    register("DiffStoreEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffStoreEvent, delay = param2)
    )
    register("DiffLoadEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffLoadEvent, delay = param2)
    )
    register("DiffArchIntRegState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchIntRegState, delay = param2)
    )
    register("DiffArchFpRegState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchFpRegState, delay = param2)
    )
    register("DiffArchVecRegState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchVecRegState, delay = param2)
    )
    register("DiffSbufferEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffSbufferEvent, delay = param2)
    )
    register("DiffIntWriteback", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffIntWriteback(param1.getOrElse(0)))
    )
    register("DiffFpWriteback", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffFpWriteback(param1.getOrElse(0)))
    )
    register("DiffVecWriteback", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffVecWriteback(param1.getOrElse(0)))
    )
  }
}
