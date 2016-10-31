/*!
 * @file SensorModuleBase.h
 * @author a_hasimoto
 * @date Date Created: 2012/May/31
 * @date Last Change:2012/May/31.
 */
#ifndef __SKL_SENSOR_MODULE_BASE_H__
#define __SKL_SENSOR_MODULE_BASE_H__

namespace skl{

/*!
 * @class SensorModuleBase
 * @brief センサ情報の取得を行うモジュールが継承すべき既定クラス
 */
template <class SensorOutput,class SensorIdentifier=std::string> class SensorModuleBase{
	public:
		SensorModuleBase();
		virtual ~SensorModuleBase();
		virtual bool open(const SensorIdentifier& sensor_identity)=0;/// センサを特定する情報を入力する
		virtual void release()=0;// センサとの接続を着る
		virtual bool isOpened()const=0;
		virtual bool grab()=0;
		virtual bool retrieve(SensorOutput& block,int channel=0)=0;
		virtual size_t size()const=0;
	protected:
		
	private:
};


/*!
 * @brief デフォルトコンストラクタ
 */
template<class SensorOutput,class SensorIdentifier> SensorModuleBase<SensorOutput,SensorIdentifier>::SensorModuleBase(){

}

/*!
 * @brief デストラクタ
 */
template<class SensorOutput,class SensorIdentifier> SensorModuleBase<SensorOutput,SensorIdentifier>::~SensorModuleBase(){
}

} // skl
#endif // __SKL_SENSOR_MODULE_BASE_H__

