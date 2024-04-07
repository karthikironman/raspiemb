import "./cards.scss";
const Card = (props) => {
  let { title = "Title", value="000.000", units="m/ms" } = props;
  return (
    <div className="card-wrapper">
      <p className="card-wrapper__title">{title}</p>
      <div className="card-wrapper__body">
        <p className="card-wrapper__body__value">{value}</p>
        <p className="card-wrapper__body__units">{units}</p>
      </div>
    </div>
  );
};
export default Card;
