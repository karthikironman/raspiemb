import { useEffect } from "react";
import "./cards.scss";
const Card = (props) => {
  let { title = "d", value=null, units="d", fontSize = 4 } = props;
  const getValue = () => {
    if(typeof value == 'number'){
        return value.toFixed(2)
    }
    return value;
  }

  const getBackgroundColor = () => {
   switch(getValue()){
    case '0.00':
      return "red"
    case '1.00':
        return "green"
    default:
          return "grey"
   }
  }
  return (
    <div className="card-wrapper" >
      <p className="card-wrapper__title">{title}</p>
      <div className="card-wrapper__body" style={{backgroundColor:getBackgroundColor()}}>
        <p className="card-wrapper__body__value" style={{fontSize:`${fontSize}rem`}}>{getValue()}</p>
        <p className="card-wrapper__body__units">{units}</p>
      </div>
    </div>
  );
};
export default Card;
